#include "AddViewDialog.hpp"
#include "ui_AddViewDialog.h"

AddViewDialog::AddViewDialog(QWidget *parent, int nMovieID) : QDialog(parent) {
    m_ui = new Ui::AddViewDialog;
    m_tags = new QList<QString>;
    m_customColumnInputList = new QList<CustomColumnLineEdit*>;
    m_ui->setupUi(this);
    this->setWindowIcon(QIcon(":/assets/Assets/Icons/Dark/plus.png"));
    m_ui->TagsInput->installEventFilter(this);

    m_ui->MovieViewDateInput->setDate(QDate::currentDate());
    m_ui->MovieReleaseYearInput->setMaximum(QDate::currentDate().year());
    m_ui->MovieReleaseYearInput->setValue(QDate::currentDate().year());
    m_ui->MovieViewDateInput->setDisplayFormat(Common::Settings->value("dateFormat").toString());

    m_tagsScrollArea = new TagsScrollArea(this);
    m_ui->FormLayout->addWidget(m_tagsScrollArea, 9, 0, 1, 2);

    m_sMovieList = GetMovieList();
    QCompleter* pMovieCompleter = new QCompleter(m_sMovieList, this);
    pMovieCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_ui->ExistingMoviesLineEdit->setCompleter(pMovieCompleter);

    m_sTagList = GetTagsList();
    QCompleter* pTagCompleter = new QCompleter(m_sTagList, this);
    pTagCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_ui->TagsInput->setCompleter(pTagCompleter);

    m_ui->ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    for(int viewType = 0 ; viewType < (int)eViewType::MaxViewType ; viewType++) {
        m_ui->ViewTypeComboBox->addItem(Common::viewTypeToQString((enum eViewType)viewType));
    }



    QObject::connect(m_ui->ExistingMoviesLineEdit, SIGNAL(textChanged(QString)), this, SLOT(MovieSearchChanged()));
    QObject::connect(m_ui->UnknownViewDateCheckbox, SIGNAL(stateChanged(int)), this, SLOT(toggleViewDateInput(int)));
    QObject::connect(m_ui->UnknownViewTypeCheckbox, SIGNAL(stateChanged(int)), this, SLOT(toggleViewTypeInput(int)));
    QObject::connect(m_ui->PosterButton, SIGNAL(clicked()), this, SLOT(SelectPoster()));
    QObject::connect(m_ui->TagsAddButton, SIGNAL(clicked()), this, SLOT(addTag()));
    QObject::connect(m_ui->TagsInput, SIGNAL(textChanged(QString)), this, SLOT(CheckTag(QString)));

    //Connectors to check if input are filled to enable Ok button
    QObject::connect(m_ui->MovieNameInput, SIGNAL(textChanged(QString)), this, SLOT(checkValid()));
    QObject::connect(m_ui->ExistingMoviesLineEdit, SIGNAL(textChanged(QString)), this, SLOT(checkValid()));

    QSqlQuery customColumnsQuery;
    if(!customColumnsQuery.exec("SELECT Name, Type, Min, Max, Precision, TextMaxLength, Optional FROM columns;"))
        Common::LogDatabaseError(&customColumnsQuery);

    QWidget* pPreviousWidget = m_ui->PosterButton; // Used for tabulation order

    int nColumnIndex = 0;
    while(customColumnsQuery.next()) {
        QLabel* columnLabel = new QLabel(customColumnsQuery.value(0).toString());
        if(customColumnsQuery.value(6).toBool() == false)
            columnLabel->setText(tr("<html><head/><body><p>%1<span style=\" color:#dd0000;\">*</span></p></body></html>").arg(customColumnsQuery.value(0).toString()));
        m_ui->CustomColumnsLabelLayout->addWidget(columnLabel, nColumnIndex);


        // Text
        CustomColumnLineEdit* input = new CustomColumnLineEdit((enum eColumnType)customColumnsQuery.value(1).toInt());
        m_customColumnInputList->append(input);
        m_ui->CustomColumnsInputLayout->addWidget(input);

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

    m_customColumnCount = nColumnIndex;



    QWidget::setTabOrder(pPreviousWidget, m_ui->TagsInput);

    // If a movie is preselected when calling the dialog
    if(nMovieID != -1) {
        QSqlQuery moviesQuery;
        if(!moviesQuery.exec("SELECT Name, ReleaseYear FROM movies WHERE ID = '" + QString::number(nMovieID) + "';"))
            Common::LogDatabaseError(&moviesQuery);
        moviesQuery.first();
        m_ui->ExistingMoviesLineEdit->setText(moviesQuery.value(0).toString() + " - " + moviesQuery.value(1).toString());
    }

    this->setFixedSize(this->sizeHint());
}

AddViewDialog::~AddViewDialog() {
    delete m_ui;
}

QStringList AddViewDialog::GetMovieList()
{
    QStringList sMovieList;
    QSqlQuery moviesQuery;
    if(!moviesQuery.exec("SELECT Name, ReleaseYear FROM movies ORDER BY Name ASC;"))
        Common::LogDatabaseError(&moviesQuery);
    while(moviesQuery.next()) {
        QString sMovie = moviesQuery.value(0).toString() + " - " + moviesQuery.value(1).toString();
        sMovieList << sMovie;
    }
    return sMovieList;
}

QStringList AddViewDialog::GetTagsList()
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

bool AddViewDialog::IsSearchedMovieAnExistingMovie() {
    return m_sMovieList.contains(m_ui->ExistingMoviesLineEdit->text());
}

QString AddViewDialog::GetSearchedMovieText()
{
    return m_ui->ExistingMoviesLineEdit->text();
}

QString AddViewDialog::getName() {
    return m_ui->MovieNameInput->text();
}
int AddViewDialog::getReleaseYear() {
    return m_ui->MovieReleaseYearInput->text().toInt();
}
enum eViewType AddViewDialog::getViewType() {
    return Common::QStringToViewType(m_ui->ViewTypeComboBox->currentText());
}
QDate AddViewDialog::getViewDate() {
    return m_ui->MovieViewDateInput->date();
}
QString AddViewDialog::getPosterPath() {
    return m_posterPath;
}
int AddViewDialog::getRating() {
    return m_ui->MovieRatingInput->value();
}

void AddViewDialog::MovieSearchChanged() {
    if(m_sMovieList.contains(m_ui->ExistingMoviesLineEdit->text()) == false)
    {
        m_ui->MovieNameInput->setEnabled(true);
        m_ui->MovieReleaseYearInput->setEnabled(true);
        m_ui->MovieRatingInput->setEnabled(true);
        m_ui->PosterButton->setEnabled(true);
        m_ui->TagsAddButton->setEnabled(true);
        m_ui->TagsInput->setEnabled(true);
        // Set state for custom columns
        for(int nWidget = 0; nWidget < m_customColumnInputList->size(); nWidget++) {
                m_customColumnInputList->at(nWidget)->setEnabled(true);
        }

        m_ui->PosterLabel->setText(tr("Poster"));
        m_posterPath = "";
    }
    else {
        m_ui->MovieNameInput->setEnabled(false);
        m_ui->MovieReleaseYearInput->setEnabled(false);
        m_ui->MovieRatingInput->setEnabled(false);
        m_ui->PosterButton->setEnabled(false);
        m_ui->TagsAddButton->setEnabled(false);
        m_ui->TagsInput->setEnabled(false);
        // Set state for custom columns
        for(int nWidget = 0; nWidget < m_customColumnInputList->size(); nWidget++) {
            m_customColumnInputList->at(nWidget)->setEnabled(false);
        }

        QString movieName = m_ui->ExistingMoviesLineEdit->text().remove(m_ui->ExistingMoviesLineEdit->text().length()-7, m_ui->ExistingMoviesLineEdit->text().length());
        QString movieYear = m_ui->ExistingMoviesLineEdit->text().remove(0, m_ui->ExistingMoviesLineEdit->text().length()-4);

        QSqlQuery posterQuery;
        if(!posterQuery.exec("SELECT Poster FROM movies WHERE Name=\""+movieName+"\" AND ReleaseYear='"+movieYear+"'"))
            Common::LogDatabaseError(&posterQuery);
        posterQuery.first();

#ifdef DEV
        QString sPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QDir::separator() + "MovieManager_Dev" + QDir::separator() + "Posters" + QDir::separator() + posterQuery.value(0).toString();
#else
        QString sPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QDir::separator() + "MovieManager" + QDir::separator() + "Posters" + QDir::separator() + posterQuery.value(0).toString();
#endif
        Common::DisplayPoster(m_ui->PosterLabel, m_ui->PosterLabel->sizeHint().height(), m_ui->PosterLabel->sizeHint().height() / (float)m_ui->PosterLabel->sizeHint().width(), sPath);
    }
}

void AddViewDialog::toggleViewDateInput(int state) {
    if(state == 2) {
        m_ui->MovieViewDateInput->setEnabled(false);
    }
    else {
        m_ui->MovieViewDateInput->setEnabled(true);
    }
}

void AddViewDialog::toggleViewTypeInput(int state) {
    if(state == 2) {
        m_ui->ViewTypeComboBox->setEnabled(false);
    }
    else {
        m_ui->ViewTypeComboBox->setEnabled(true);
    }
}

bool AddViewDialog::isDateUnknown() {
    return m_ui->UnknownViewDateCheckbox->isChecked();
}

bool AddViewDialog::isTypeUnknown() {
    return m_ui->UnknownViewTypeCheckbox->isChecked();
}


void AddViewDialog::checkValid()
{
    bool bIsValid = true;
    //If movie isn't selected in combobox
    if(m_sMovieList.contains(m_ui->ExistingMoviesLineEdit->text()) == false) {
        if(m_ui->MovieNameInput->text() == "") {
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
    }

    m_ui->ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(bIsValid);
}

void AddViewDialog::SelectPoster() {
    float ratio = (float)m_ui->PosterLabel->sizeHint().height() / (float)m_ui->PosterLabel->sizeHint().width();
    m_posterPath = Common::SelectPoster(this);
    if(m_posterPath != "")
        Common::DisplayPoster(m_ui->PosterLabel, m_ui->PosterLabel->sizeHint().height(), ratio, m_posterPath);
}

void AddViewDialog::addTag() {
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

QList<QString>* AddViewDialog::getTags() {
    return m_tags;
}

void AddViewDialog::clickedTag(Tag* tag) {
    for(int i = 0 ; i < m_tags->length() ; i++) {
        if(QString::compare(tag->getSavedTag(), m_tags->at(i)) == 0) {
            m_tags->removeAt(i);
        }
    }

    delete tag;

    CheckTag(m_ui->TagsInput->text());
}

void AddViewDialog::mouseEnteredTag(Tag* tag) {
    tag->setSavedTag(tag->text());
    tag->setText("❌");
}

void AddViewDialog::mouseLeftTag(Tag* tag) {
    tag->setText(tag->getSavedTag());
}

CustomColumnLineEdit* AddViewDialog::getCustomColumnInputAt(int nIndex) {
    if(nIndex < m_customColumnInputList->size() && nIndex >= 0)
        return m_customColumnInputList->at(nIndex);
    else
        return nullptr;
}

int AddViewDialog::getCustomColumnCount() {
    return m_customColumnCount;
}

bool AddViewDialog::eventFilter(QObject *obj, QEvent *event) {
    if (obj == m_ui->TagsInput && event->type() == QEvent::KeyPress) {
        QKeyEvent* key = static_cast<QKeyEvent*>(event);
        if(key->key() == Qt::Key_Return && m_ui->TagsAddButton->isEnabled() == true) {
            addTag();
            return true;
        }
    }
    return false;
}

void AddViewDialog::CheckTag(QString sTag)
{
    if(m_tags->contains(sTag) == true)
        m_ui->TagsAddButton->setEnabled(false);
    else
        m_ui->TagsAddButton->setEnabled(true);
}
