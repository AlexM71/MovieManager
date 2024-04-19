#include "Common.h"

Log* Common::Log = new class Log();
#ifdef DEV
    QSettings* Common::Settings = new class QSettings("MovieManager_Dev", "MovieManager_Dev");
#else
    QSettings* Common::Settings = new class QSettings("MovieManager", "MovieManager");
#endif

Common::Common()
{

}

QString Common::SelectPoster(QWidget* parent) {
    QList ValidExtensionList = {"jpg", "jpeg", "png", "webp"};
    QString sPath;
    bool bValidImage = false;
    while (bValidImage == false) {
        bValidImage = true;
        sPath = QFileDialog::getOpenFileName(parent, QObject::tr("Select a picture"), QString(), QObject::tr("Image (*.png; *.jpg; *.jpeg; *.webp)"));
        if(sPath.size() <= 0)
            return "";

        QString sFileExt = sPath;
        sFileExt.remove(0, sPath.lastIndexOf(".")+1);
        // Test if file extension is valid
        if(ValidExtensionList.contains(sFileExt) == false) {
            QMessageBox::critical(parent, QObject::tr("Incorrect format"), QObject::tr("Image format is incorrect\nValid formats: jpg, jpeg, png, webp"));
            bValidImage = false;
        }
    }
    return sPath;
}

void Common::DisplayPoster(QLabel* poster, int nPosterHeight, float fSafeRatio, QString sPath) {
    QPixmap pixmap;
    //If no movie has no cover
    QFileInfo imgFile(sPath);
    if(imgFile.exists(sPath) && imgFile.isFile()) {
        pixmap.load(sPath);
    }
    else {
        pixmap.load(":/assets/Assets/nocover.png");
    }
    QPixmap pm;

    //If picture is too wide, poster is scaled to width to fit in UI
    if((float)pixmap.height()/(float)pixmap.width() < fSafeRatio) {
        pm = pixmap.scaledToWidth(nPosterHeight/fSafeRatio, Qt::SmoothTransformation);
    }
    else {
        pm = pixmap.scaledToHeight(nPosterHeight, Qt::SmoothTransformation);
    }

    poster->setPixmap(pm);
}

void Common::ratingToStar(int rating, QLabel* ratingLabel) {

    ratingLabel->setText("");
    QPixmap* pixmap;

    switch(rating) {
        case 0:
            pixmap = new QPixmap(":/assets/Assets/stars/0.png");
            break;
        case 1:
            pixmap = new QPixmap(":/assets/Assets/stars/1.png");
            break;
        case 2:
            pixmap = new QPixmap(":/assets/Assets/stars/2.png");
            break;
        case 3:
            pixmap = new QPixmap(":/assets/Assets/stars/3.png");
            break;
        case 4:
            pixmap = new QPixmap(":/assets/Assets/stars/4.png");
            break;
        case 5:
            pixmap = new QPixmap(":/assets/Assets/stars/5.png");
            break;
        case 6:
            pixmap = new QPixmap(":/assets/Assets/stars/6.png");
            break;
        case 7:
            pixmap = new QPixmap(":/assets/Assets/stars/7.png");
            break;
        case 8:
            pixmap = new QPixmap(":/assets/Assets/stars/8.png");
            break;
        case 9:
            pixmap = new QPixmap(":/assets/Assets/stars/9.png");
            break;
        case 10:
            pixmap = new QPixmap(":/assets/Assets/stars/10.png");
            break;
        default:
            pixmap = new QPixmap();
            break;
    }

    QPixmap resizedPixmap = pixmap->scaledToHeight(20, Qt::SmoothTransformation);
    ratingLabel->setPixmap(resizedPixmap);
}

void Common::clearLayout(QLayout* layout) {
    if (layout == NULL)
        return;
    QLayoutItem *item;
    while((item = layout->takeAt(0))) {
        if (item->layout()) {
            clearLayout(item->layout());
            delete item->layout();
        }
        if (item->widget()) {
           delete item->widget();
        }
        delete item;
    }
}

QString Common::viewTypeToQString(enum eViewType type) {
    if(type == eViewType::Bluray) {
        return QObject::tr("Blu-Ray");
    }
    if(type == eViewType::Television) {
        return QObject::tr("TV");
    }
    if(type == eViewType::Cinema) {
        return QObject::tr("Cinema");
    }
    if(type == eViewType::CinemaReplay) {
        return QObject::tr("Cinema (replay)");
    }
    if(type == eViewType::DVD) {
        return QObject::tr("DVD");
    }
    if(type == eViewType::Streaming) {
        return QObject::tr("Streaming");
    }
    if(type == eViewType::Download) {
        return QObject::tr("Download");
    }
    if(type == eViewType::VHS) {
        return QObject::tr("VHS");
    }
    if(type == eViewType::Other) {
        return QObject::tr("Other physical support");
    }
    if(type == eViewType::Unknown) {
        return QObject::tr("Unknown");
    }
    if(type == eViewType::UHD) {
        return QObject::tr("Blu-Ray 4K");
    }
    else {
        qDebug() << "Error";
        //Log(QObject::tr("Unknown view type"), Error);
        return QObject::tr("Unknown");
    }
}

enum eViewType Common::QStringToViewType(QString type) {
    if(QString::compare(type, QObject::tr("Blu-Ray")) == 0) {
        return eViewType::Bluray;
    }
    else if(QString::compare(type, QObject::tr("TV")) == 0) {
        return eViewType::Television;
    }
    else if(QString::compare(type, QObject::tr("Cinema")) == 0) {
        return eViewType::Cinema;
    }
    else if(QString::compare(type, QObject::tr("Cinema (replay)")) == 0) {
        return eViewType::CinemaReplay;
    }
    else if(QString::compare(type, QObject::tr("DVD")) == 0) {
        return eViewType::DVD;
    }
    else if(QString::compare(type, QObject::tr("Streaming")) == 0) {
        return eViewType::Streaming;
    }
    else if(QString::compare(type, QObject::tr("Download")) == 0) {
        return eViewType::Download;
    }
    else if(QString::compare(type, QObject::tr("VHS")) == 0) {
        return eViewType::VHS;
    }
    else if(QString::compare(type, QObject::tr("Other physical support")) == 0) {
        return eViewType::Other;
    }
    else if(QString::compare(type, QObject::tr("Unknown")) == 0) {
        return eViewType::Unknown;
    }
    else if(QString::compare(type, QObject::tr("Blu-Ray 4K")) == 0) {
        return eViewType::UHD;
    }
    else {
        return eViewType::MaxViewType;
    }
}

QString Common::ColumnTypeToQString(enum eColumnType type) {
    switch(type) {
        case eColumnType::Double:
            return QObject::tr("Precise integer");
        case eColumnType::Integer:
            return QObject::tr("Integer");
        case eColumnType::Text:
        default:
            return QObject::tr("Text");
    }
}

void Common::setIconAccordingToTheme(QAction* action, QString filename) {
    if(isThemeBright((enum eTheme)Common::Settings->value("theme").toInt())) {
        action->setIcon(QIcon(":/assets/Assets/Icons/Dark/"+filename));
    }
    else {
        action->setIcon(QIcon(":/assets/Assets/Icons/Bright/"+filename));
    }
}

void Common::setIconAccordingToTheme(QPushButton* action, QString filename) {
    if(isThemeBright((enum eTheme)Common::Settings->value("theme").toInt())) {
        action->setIcon(QIcon(":/assets/Assets/Icons/Dark/"+filename));
    }
    else {
        action->setIcon(QIcon(":/assets/Assets/Icons/Bright/"+filename));
    }
}

bool Common::isThemeBright(enum eTheme eTheme)
{
    switch(eTheme)
    {
        case eTheme::System:
            if(qApp != nullptr)
            {
                if(qApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark)
                    return false;
               else
                    return true;
            }
            else
                return true;
        case eTheme::Dark:
        case eTheme::MidnightPurple:
        case eTheme::OLED:
            return false;
        case eTheme::Classic:
        default:
            return true;
    }
}

QString Common::getVersion() {
    return "1.2.1";
}

void Common::LogDatabaseError(QSqlQuery *query) {
    QString sLog = QObject::tr("Database error: %1\nQuery: %2").arg(query->lastError().text(), query->executedQuery());
    Common::Log->append(sLog, eLog::Error);
}

