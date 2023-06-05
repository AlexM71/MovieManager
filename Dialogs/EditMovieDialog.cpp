#include "EditMovieDialog.h"
#include "ui_EditMovieDialog.h"

EditMovieDialog::EditMovieDialog(QString ID, QWidget *parent) : QDialog(parent) {
    m_ui = new Ui::EditMovieDialog;
    m_tags = new QList<QString>;
    m_ui->setupUi(this);
    this->setFixedSize(500,300);
    m_ID = &ID;
    this->setWindowIcon(QIcon(":/assets/Assets/Icons/Dark/edit.png"));

    QSqlQuery movieQuery;
    if(!movieQuery.exec("SELECT Name, ReleaseYear, Entries, Rating, Poster FROM movies WHERE ID='"+*m_ID+"'"))
        Common::LogDatabaseError(&movieQuery);
    movieQuery.first();

    m_posterPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "\\MovieManager\\Posters\\"+movieQuery.value(4).toString();
    loadPoster(m_posterPath);

    QSqlQuery tagsQuery;
    if(!tagsQuery.exec("SELECT Tag FROM tags WHERE ID_Movie='"+*m_ID+"'"))
        Common::LogDatabaseError(&tagsQuery);
    while(tagsQuery.next()) {
        m_tags->append(tagsQuery.value(0).toString());
        Tag* tag = new Tag(tagsQuery.value(0).toString());

        QObject::connect(tag, SIGNAL(clicked(Tag*)), this, SLOT(clickedTag(Tag*)));
        QObject::connect(tag, SIGNAL(mouseEnter(Tag*)), this, SLOT(mouseEnteredTag(Tag*)));
        QObject::connect(tag, SIGNAL(mouseLeave(Tag*)), this, SLOT(mouseLeftTag(Tag*)));

        m_ui->TagsLayout->insertWidget(m_ui->TagsLayout->count()-1,tag,0,Qt::AlignLeft);
    }

    this->setWindowTitle(tr("Edit - %1").arg(movieQuery.value(0).toString()));

    m_ui->ReleaseYearInput->setMaximum(QDate::currentDate().year());

    m_ui->NameInput->setText(movieQuery.value(0).toString());
    m_ui->ReleaseYearInput->setValue(movieQuery.value(1).toInt());
    if(movieQuery.value(2).toInt() == -1) {
        m_ui->UnknownEntriesCheckbox->setChecked(true);
        m_ui->EntriesInput->setEnabled(false);
        m_ui->EntriesInput->setValue(0);
    }
    else {
        m_ui->EntriesInput->setValue(movieQuery.value(2).toInt());
    }
    m_ui->RatingInput->setValue(movieQuery.value(3).toInt());

    QObject::connect(m_ui->PosterButton, SIGNAL(clicked()), this, SLOT(loadPoster()));
    QObject::connect(m_ui->TagsAddButton, SIGNAL(clicked()), this, SLOT(addTag()));
    QObject::connect(m_ui->UnknownEntriesCheckbox, SIGNAL(stateChanged(int)), this, SLOT(toggleEntriesInput(int)));
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

int EditMovieDialog::getEntries() {
    return m_ui->EntriesInput->value();
}

void EditMovieDialog::loadPoster(QString path) {
    QString tmp = m_posterPath;
    Common::loadPoster(this, m_ui->PosterLabel, 150, 1.33, path, &m_posterPath);
    if (QString::compare(tmp, m_posterPath) != 0) {
        m_newPoster = true;
    }
}

void EditMovieDialog::addTag() {
    if(m_ui->TagsInput->text().length() != 0) {
        m_tags->append(m_ui->TagsInput->text());
        Tag* tag = new Tag(m_ui->TagsInput->text());

        m_ui->TagsLayout->insertWidget(m_ui->TagsLayout->count()-1,tag,0,Qt::AlignLeft);
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
    int width = tag->width();
    tag->setSavedTag(tag->text());
    tag->setText("❌");
    tag->setMinimumWidth(width);
}

void EditMovieDialog::mouseLeftTag(Tag* tag) {
    tag->setMinimumWidth(31);
    tag->setText(tag->getSavedTag());
}

bool EditMovieDialog::isEntriesUnknown() {
    return m_ui->UnknownEntriesCheckbox->isChecked();
}

void EditMovieDialog::toggleEntriesInput(int state) {
    if(state == 2) {
        m_ui->EntriesInput->setEnabled(false);
    }
    else {
        m_ui->EntriesInput->setEnabled(true);
    }
}
