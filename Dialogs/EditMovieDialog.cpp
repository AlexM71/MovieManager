#include "EditMovieDialog.hpp"
#include "ui_EditMovieDialog.h"

EditMovieDialog::EditMovieDialog(QString ID, QWidget *parent) : QDialog(parent) {
    m_ui = new Ui::EditMovieDialog;
    m_tags = new QList<QString>;
    m_customColumnInputList = new QList<CustomColumnLineEdit*>;
    m_ui->setupUi(this);
    m_ID = &ID;
    this->setWindowIcon(QIcon(":/assets/Assets/Icons/Dark/edit.png"));
    m_ui->TagsInput->installEventFilter(this);

    m_tagsScrollArea = new TagsScrollArea(this);
    m_ui->FormLayout->addWidget(m_tagsScrollArea, 6, 0, 1, 2);

    QSqlQuery movieQuery;
    if(!movieQuery.exec("SELECT Name, ReleaseYear, Rating, Poster FROM movies WHERE ID='"+*m_ID+"'"))
        Common::LogDatabaseError(&movieQuery);
    movieQuery.first();

#ifdef DEV
    m_posterPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QDir::separator() + "MovieManager_Dev" + QDir::separator() + "Posters" + QDir::separator() + movieQuery.value(3).toString();
#else
    m_posterPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QDir::separator() + "MovieManager" + QDir::separator() + "Posters" + QDir::separator() + movieQuery.value(3).toString();
#endif

    float ratio = (float)m_ui->PosterLabel->sizeHint().height() / (float)m_ui->PosterLabel->sizeHint().width();
    Common::DisplayPoster(m_ui->PosterLabel, m_ui->PosterLabel->sizeHint().height(), ratio, m_posterPath);

    this->setWindowTitle(tr("Edit - %1").arg(movieQuery.value(0).toString()));

    m_ui->ReleaseYearInput->setMaximum(QDate::currentDate().year());

    m_ui->NameInput->setText(movieQuery.value(0).toString());
    m_ui->ReleaseYearInput->setValue(movieQuery.value(1).toInt());
    m_ui->RatingInput->setValue(movieQuery.value(2).toInt());

    QObject::connect(m_ui->PosterButton, SIGNAL(clicked()), this, SLOT(SelectPoster()));
    QObject::connect(m_ui->TagsAddButton, SIGNAL(clicked()), this, SLOT(addTag()));

    //Connectors to check if input are filled to enable Ok button
    QObject::connect(m_ui->NameInput, SIGNAL(textChanged(QString)), this, SLOT(checkValid()));

    QSqlQuery customColumnsQuery;
    QString sCustomColumns;
    int nCustomColumnCount = 0;

    if(!customColumnsQuery.exec("SELECT Name, Type, Min, Max, Precision, TextMaxLength, Optional FROM columns;"))
        Common::LogDatabaseError(&customColumnsQuery);

    QWidget* pPreviousWidget = m_ui->PosterButton; // Used for tabulation order

    int nColumnIndex = 0;
    while(customColumnsQuery.next()) {
        QLabel* columnLabel = new QLabel(customColumnsQuery.value(0).toString());
        if(customColumnsQuery.value(6).toBool() == false)
            columnLabel->setText(tr("<html><head/><body><p>%1<span style=\" color:#dd0000;\">*</span></p></body></html>").arg(customColumnsQuery.value(0).toString()));

        m_ui->CustomColumnsLabelLayout->addWidget(columnLabel, nColumnIndex);

        sCustomColumns.append(" \"" + customColumnsQuery.value(0).toString() + "\",");
        nCustomColumnCount++;

        // Text
        CustomColumnLineEdit* input = new CustomColumnLineEdit((enum eColumnType)customColumnsQuery.value(1).toInt());
        m_ui->CustomColumnsInputLayout->addWidget(input);
        m_customColumnInputList->append(input);

        QWidget::setTabOrder(pPreviousWidget, input);
        pPreviousWidget = input;

        QObject::connect(input, SIGNAL(textChanged(QString)), this, SLOT(checkValid()));

        input->setLabel(customColumnsQuery.value(0).toString());
        input->setMin(customColumnsQuery.value(2).toDouble());
        input->setMax(customColumnsQuery.value(3).toDouble());
        input->setPrecision(customColumnsQuery.value(4).toInt());
        input->setTextMaxLength(customColumnsQuery.value(5).toInt());
        input->setOptional(customColumnsQuery.value(6).toBool());

        nColumnIndex++;
    }

    QWidget::setTabOrder(pPreviousWidget, m_ui->TagsInput);
    sCustomColumns.removeLast(); // Removes the last ","

    m_customColumnCount = nColumnIndex;

    // Add custom columns informations
    QString sRequest = "SELECT" + sCustomColumns + " FROM movies WHERE ID='"+ID+"'";
    QSqlQuery customColumnsInformationsQuery;
    if(!customColumnsInformationsQuery.exec(sRequest))
        Common::LogDatabaseError(&customColumnsInformationsQuery);
    customColumnsInformationsQuery.first();

    for(int nColumn = 0; nColumn < nCustomColumnCount; nColumn++)
    {
        if(m_customColumnInputList->at(nColumn) != nullptr)
        {
            CustomColumnLineEdit* input = m_customColumnInputList->at(nColumn);
            input->setText(customColumnsInformationsQuery.value(nColumn).toString());
        }
    }

    this->setFixedSize(this->sizeHint());

    QLayoutItem* spacer = m_tagsScrollArea->widget()->layout()->takeAt(m_tagsScrollArea->widget()->layout()->count()-1);

    QSqlQuery tagsQuery;
    if(!tagsQuery.exec("SELECT Tag FROM tags WHERE ID_Movie='"+*m_ID+"'"))
        Common::LogDatabaseError(&tagsQuery);
    while(tagsQuery.next()) {
        m_tags->append(tagsQuery.value(0).toString());
        Tag* tag = new Tag(tagsQuery.value(0).toString());

        QObject::connect(tag, SIGNAL(clicked(Tag*)), this, SLOT(clickedTag(Tag*)));
        QObject::connect(tag, SIGNAL(mouseEnter(Tag*)), this, SLOT(mouseEnteredTag(Tag*)));
        QObject::connect(tag, SIGNAL(mouseLeave(Tag*)), this, SLOT(mouseLeftTag(Tag*)));

        m_tagsScrollArea->widget()->layout()->addWidget(tag);
    }
    m_tagsScrollArea->widget()->layout()->addItem(spacer);

    m_sTagList = GetTagsList();
    QCompleter* pTagCompleter = new QCompleter(m_sTagList, this);
    pTagCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_ui->TagsInput->setCompleter(pTagCompleter);

    QObject::connect(m_ui->TagsInput, SIGNAL(textChanged(QString)), this, SLOT(CheckTag(QString)));

    checkValid();
}

QStringList EditMovieDialog::GetTagsList()
{
    QStringList sTagsList;
    QSqlQuery tagsQuery;
    if(!tagsQuery.exec("SELECT Tag FROM tags ORDER BY Tag ASC;"))
        Common::LogDatabaseError(&tagsQuery);
    while(tagsQuery.next()) {
        QString sTag = tagsQuery.value(0).toString();
        if(sTagsList.contains(sTag) == false)
            sTagsList << sTag;
    }
    return sTagsList;
}

EditMovieDialog::~EditMovieDialog() {
    delete m_ui;
}

bool EditMovieDialog::edited() {
    return m_edited;
}

bool EditMovieDialog::newPoster() {
    return m_newPoster;
}

QString EditMovieDialog::getMovieName() {
    return m_ui->NameInput->text();
}

QString EditMovieDialog::getReleaseYear() {
     return m_ui->ReleaseYearInput->text();
}

QString EditMovieDialog::getPosterPath() {
    return m_posterPath;
}

int EditMovieDialog::getRating() {
    return m_ui->RatingInput->value();
}

void EditMovieDialog::SelectPoster() {
    QString sNewPath = m_posterPath;
    sNewPath = Common::SelectPoster(m_ui->PosterLabel);
    if(QString::compare(sNewPath, m_posterPath) != 0 && sNewPath.length() != 0) {
        m_newPoster = true;
        m_posterPath = sNewPath;
        float ratio = (float)m_ui->PosterLabel->sizeHint().height() / (float)m_ui->PosterLabel->sizeHint().width();
        Common::DisplayPoster(m_ui->PosterLabel, m_ui->PosterLabel->sizeHint().height(), ratio, m_posterPath);
    }
}

void EditMovieDialog::addTag() {
    if(m_ui->TagsInput->text().length() != 0) {
        m_tags->append(m_ui->TagsInput->text());
        Tag* tag = new Tag(m_ui->TagsInput->text());

        QLayoutItem* spacer = m_tagsScrollArea->widget()->layout()->takeAt(m_tagsScrollArea->widget()->layout()->count()-1);
        m_tagsScrollArea->widget()->layout()->addWidget(tag);
        m_tagsScrollArea->widget()->layout()->addItem(spacer);
        m_ui->TagsInput->clear();

        QObject::connect(tag, SIGNAL(clicked(Tag*)), this, SLOT(clickedTag(Tag*)));
        QObject::connect(tag, SIGNAL(mouseEnter(Tag*)), this, SLOT(mouseEnteredTag(Tag*)));
        QObject::connect(tag, SIGNAL(mouseLeave(Tag*)), this, SLOT(mouseLeftTag(Tag*)));
    }
}

QList<QString>* EditMovieDialog::getTags() {
    return m_tags;
}

void EditMovieDialog::clickedTag(Tag* tag) {

    for(int i = 0 ; i < m_tags->length() ; i++) {
        if(QString::compare(tag->getSavedTag(), m_tags->at(i)) == 0) {
            m_tags->removeAt(i);
        }
    }

    delete tag;
}

void EditMovieDialog::mouseEnteredTag(Tag* tag) {
    tag->setSavedTag(tag->text());
    tag->setText("❌");
}

void EditMovieDialog::mouseLeftTag(Tag* tag) {
    tag->setText(tag->getSavedTag());
}

CustomColumnLineEdit* EditMovieDialog::getCustomColumnInputAt(int nIndex) {
    if(nIndex < m_customColumnInputList->size() && nIndex >= 0)
        return m_customColumnInputList->at(nIndex);
    else
        return nullptr;
}


int EditMovieDialog::getCustomColumnCount() {
    return m_customColumnCount;
}

bool EditMovieDialog::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_ui->TagsInput && event->type() == QEvent::KeyPress) {
        QKeyEvent* key = static_cast<QKeyEvent*>(event);
        if(key->key() == Qt::Key_Return && m_ui->TagsAddButton->isEnabled() == true) {
            addTag();
            return true;
        }
    }
    return false;
}

void EditMovieDialog::checkValid()
{
    bool bIsValid = true;
    if(m_ui->NameInput->text() == "") {
        bIsValid = false;
    }
    for(int nCustomColumn = 0; nCustomColumn < m_customColumnInputList->length(); nCustomColumn++)
    {
        // If column is not optional and text is empty -> not valid
        if(m_customColumnInputList->at(nCustomColumn)->IsOptional() == false)
        {
            if(m_customColumnInputList->at(nCustomColumn)->text() == "")
            {
                bIsValid = false;
                break;
            }
        }
    }

    m_ui->ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(bIsValid);
}

void EditMovieDialog::CheckTag(QString sTag)
{
    if(m_tags->contains(sTag) == true)
        m_ui->TagsAddButton->setEnabled(false);
    else
        m_ui->TagsAddButton->setEnabled(true);
}
