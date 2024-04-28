#include "MainWindow.hpp"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QApplication* app) {

#ifdef DEV
    m_savepath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QDir::separator() + "MovieManager_Dev";
    this->setWindowTitle("MovieManager (DEV)");
#else
    m_savepath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QDir::separator() + "MovieManager";
    this->setWindowTitle(tr("MovieManager"));
#endif

    QDir dir(m_savepath);
    dir.mkpath("./Posters");

    m_app = app;
    m_ui = new Ui::MainWindow;
    m_ui->setupUi(this);
    m_customColumnsRequestFilter = "";

    m_selectedTagsScrollArea = new TagsScrollArea();
    QLayoutItem* movieList = m_ui->MoviesLayout->takeAt(1);
    QLayoutItem* actionLayout = m_ui->MoviesLayout->takeAt(1);
    m_ui->MoviesLayout->addWidget(m_selectedTagsScrollArea);
    m_ui->MoviesLayout->addItem(movieList);
    m_ui->MoviesLayout->addItem(actionLayout);

    m_movieTagsScrollArea = new TagsScrollArea();
    m_ui->MovieInfoLayout->addWidget(m_movieTagsScrollArea, 12, 0, 1, 2);

    // Expand movie's list
    m_ui->MoviesLayout->setStretch(2,100);

    m_app->setWindowIcon(QIcon(":/assets/Assets/logo.png"));

    m_selectedTagsScrollArea->setMaximumHeight(0);

    // Shhhh, keep it secret
    if(QString::compare(m_app->arguments().at(0), "Neo")) {
        m_ui->EasterEggAct->setVisible(false);
    }

    // Set default settings value if not already set
    if(Common::Settings->contains("language") == false)
        Common::Settings->setValue("language", 0);
    if(Common::Settings->contains("theme") == false)
        Common::Settings->setValue("theme", 0);
    if(Common::Settings->contains("matrixMode") == false)
        Common::Settings->setValue("matrixMode", false);
    if(Common::Settings->contains("quickSearchCaseSensitive") == false)
        Common::Settings->setValue("quickSearchCaseSensitive", false);
    if(Common::Settings->contains("moreLogs") == false)
        Common::Settings->setValue("moreLogs", false);
    if(Common::Settings->contains("dateFormat") == false)
        Common::Settings->setValue("dateFormat", "yyyy-MM-dd");
    if(Common::Settings->contains("LastMovieOpened") == false)
        Common::Settings->setValue("LastMovieOpened", 0);

    if(InitDatabase() == true)
    {
        if(getDatabaseVersion() == "")
            CreateTables();

        CheckDatabaseVersion();

        if(BackupDatabase() == true)
            Common::Log->append(tr("Database backup created successfully"), eLog::Success);
    }
    else
    {
        QMessageBox::critical(this, tr("Database error"), tr("Failed to open the database, please check for assistance"));
    }

    refreshLanguage();
    refreshTheme();

    m_ui->MoviesListWidget->setHorizontalHeaderLabels(QStringList() << tr("Name") << tr("Release year"));


    fillTable();

    menuBarConnectors();

    m_ui->MoviesListWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_ui->MoviesListWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    m_ui->MoviesListWidget->setColumnWidth(1,100);
    m_ui->MoviesListWidget->setColumnHidden(2, true);

    // Set current row on last movie displayed before last close
    for(int nIndex = 0; nIndex < m_ui->MoviesListWidget->rowCount(); nIndex++)
    {
        if(m_ui->MoviesListWidget->item(nIndex, 2)->text().toInt() == Common::Settings->value("LastMovieOpened").toInt())
        {
            m_ui->MoviesListWidget->setCurrentCell(nIndex, 0);
            fillMovieInfos(Common::Settings->value("LastMovieOpened").toInt());
            break;
        }
    }

    CheckForUpdates(false);

    QObject::connect(m_ui->AddViewButton, SIGNAL(clicked()), this, SLOT(addView()));
    QObject::connect(m_ui->AdvancedSearchButton, SIGNAL(clicked()), this, SLOT(openFilters()));
    QObject::connect(m_ui->ResetFiltersButton, SIGNAL(clicked()), this, SLOT(resetFilters()));
    QObject::connect(m_ui->MoviesListWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customMenuRequested(QPoint)));
    QObject::connect(m_ui->QuickSearchLineEdit, SIGNAL(textChanged(QString)), this, SLOT(fillTable()));
    QObject::connect(qApp->styleHints(), SIGNAL(colorSchemeChanged(Qt::ColorScheme)), this, SLOT(refreshTheme()));
}

MainWindow::~MainWindow() {
    delete m_ui;
}

void MainWindow::closeEvent([[maybe_unused]] QCloseEvent* event ) {
    if(m_ui->MoviesListWidget->currentRow() != -1)
        Common::Settings->setValue("LastMovieOpened", m_ui->MoviesListWidget->item(m_ui->MoviesListWidget->currentRow(),2)->text().toInt());
}

// Creates a backup of the database
bool MainWindow::BackupDatabase()
{
    QString sTempDatabase = m_savepath + QDir::separator() + "movieDatabase.TEMP.db";
    QString sBackupDatabase = m_savepath + QDir::separator() + "movieDatabase.BACKUP.db";

    // Creates a backup of the database with suffix TEMP
    if(QFile::copy(m_savepath + QDir::separator() + "movieDatabase.db", sTempDatabase) == false)
    {
        Common::Log->append(tr("Failed to create a backup of the database (error 1)"), eLog::Error);
        return false;
    }

    // Removes the backup if exists
    if(QFile::exists(sBackupDatabase))
    {
        if(QFile::remove(sBackupDatabase) == false)
        {
            Common::Log->append(tr("Failed to create a backup of the database (error 2)"), eLog::Error);
            if(QFile::remove(sTempDatabase) == false)
                Common::Log->append(tr("Failed to create a backup of the database (error 3)"), eLog::Error);
            return false;
        }
    }

    // Renames the backup suffix from TEMP to BACKUP
    if(QFile::rename(sTempDatabase, sBackupDatabase) == false)
    {
        Common::Log->append(tr("Failed to create a backup of the database (error 4)"), eLog::Error);
        if(QFile::remove(sTempDatabase) == false)
            Common::Log->append(tr("Failed to create a backup of the database (error 5)"), eLog::Error);
        return false;
    }

    return true;
}

bool MainWindow::InitDatabase() {
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(m_savepath + QDir::separator() + "movieDatabase.db");

    if(!m_db.open()) {
        Common::Log->append(tr("Can't open database"), eLog::Error);
        return false;
    }
    else {
        Common::Log->append(tr("Database opened successfully"), eLog::Success);
        return true;
    }
}

void MainWindow::CreateTables()
{
    //Movies table
    QString movieDatabaseCreationString = "CREATE TABLE IF NOT EXISTS movies ("
                                   "ID          INTEGER PRIMARY KEY AUTOINCREMENT,"
                                   "Name        TEXT,"
                                   "ReleaseYear INTEGER,"
                                   "Rating      INTEGER,"
                                   "Poster      TEXT);";

    QSqlQuery movieDBQuery;

    if(!movieDBQuery.exec(movieDatabaseCreationString)) {
        Common::LogDatabaseError(&movieDBQuery);
    }

    //Views table
    QString ViewsDatabaseCreationString = "CREATE TABLE IF NOT EXISTS views ("
                                   "ID          INTEGER PRIMARY KEY AUTOINCREMENT,"
                                   "ID_Movie    INTEGER,"
                                   "ViewDate    DATE,"
                                   "ViewType    INTEGER);";

    QSqlQuery viewsBDQuery;

    if(!viewsBDQuery.exec(ViewsDatabaseCreationString)) {
        Common::LogDatabaseError(&viewsBDQuery);
    }

    //TagsInfo Table
    QString TagsInfoDatabaseCreationString = "CREATE TABLE IF NOT EXISTS tagsInfo ("
                                   "Tag         TEXT PRIMARY KEY ,"
                                   "Color       TEXT);";

    QSqlQuery tagsInfoBDQuery;

    if(!tagsInfoBDQuery.exec(TagsInfoDatabaseCreationString)) {
        Common::LogDatabaseError(&tagsInfoBDQuery);
    }

    //Tags Table
    QString TagsDatabaseCreationString = "CREATE TABLE IF NOT EXISTS tags ("
                                   "ID_Movie    INTEGER,"
                                   "Tag         TEXT);";

    QSqlQuery TagsBDQuery;

    if(!TagsBDQuery.exec(TagsDatabaseCreationString)) {
        Common::LogDatabaseError(&TagsBDQuery);
    }

    //Columns Table
    QString ColumnDatabaseCreationString = "CREATE TABLE IF NOT EXISTS columns ("
                                         "Name          TEXT,"
                                         "Type          INTEGER,"
                                         "Min           REAL,"
                                         "Max           REAL,"
                                         "Precision     INTEGER,"
                                         "TextMaxLength INTEGER, "
                                         "Optional      INTEGER);";

    QSqlQuery ColumnBDQuery;

    if(!ColumnBDQuery.exec(ColumnDatabaseCreationString)) {
        Common::LogDatabaseError(&ColumnBDQuery);
    }

    // Version Table
    QString VersionDatabaseCreationString = "CREATE TABLE IF NOT EXISTS version ("
                                            "Version    Text);";

    QSqlQuery VersionBDQuery;

    if(!VersionBDQuery.exec(VersionDatabaseCreationString)) {
        Common::LogDatabaseError(&VersionBDQuery);
    }

    QString VersionDatabaseInsert = "INSERT INTO Version (Version) VALUES (\"1.2.1\");";

    if(!VersionBDQuery.exec(VersionDatabaseInsert)) {
        Common::LogDatabaseError(&VersionBDQuery);
    }
}

void MainWindow::fillTable() {

    m_ui->MoviesListWidget->blockSignals(true);
    m_ui->MoviesListWidget->setSortingEnabled(false);

    int nSavedSelectedMovieID = -1;
    // If a row is selected, we temporarily save its movie ID
    if(m_ui->MoviesListWidget->currentRow() != -1)
        nSavedSelectedMovieID = m_ui->MoviesListWidget->item(m_ui->MoviesListWidget->currentRow(),2)->text().toInt();

    //Clear the table
    int movieListRowCount = m_ui->MoviesListWidget->rowCount();
    for(int i=movieListRowCount ; i >= 0 ; i--) {
        m_ui->MoviesListWidget->removeRow(i);
    }

    //Fetch every unique movies
    QSqlQuery moviesQuery;
    QString sMovieQueryRequest = "SELECT ID, Name, ReleaseYear FROM movies";
    sMovieQueryRequest.append(m_customColumnsRequestFilter);

    if(Common::Settings->value("moreLogs").toBool() == true)
        Common::Log->append(tr("Fetching from database..."), eLog::Notice);

    if(!moviesQuery.exec(sMovieQueryRequest))
        Common::LogDatabaseError(&moviesQuery);

    int numberOfParsedMovies = 0;
    while(moviesQuery.next()) {
        QTableWidgetItem* name = new QTableWidgetItem();
        QTableWidgetItem* releaseYear = new QTableWidgetItem();
        QTableWidgetItem* ID = new QTableWidgetItem();

        releaseYear->setTextAlignment(Qt::AlignCenter);

        ID->setText(moviesQuery.value(0).toString());
        name->setText(moviesQuery.value(1).toString());
        releaseYear->setText(moviesQuery.value(2).toString());

        //Creates a new row on the table
        m_ui->MoviesListWidget->insertRow(m_ui->MoviesListWidget->rowCount());

        //Insert QTableWidgetItem in the table
        m_ui->MoviesListWidget->setItem(m_ui->MoviesListWidget->rowCount()-1, 0, name);
        m_ui->MoviesListWidget->setItem(m_ui->MoviesListWidget->rowCount()-1, 1, releaseYear);
        m_ui->MoviesListWidget->setItem(m_ui->MoviesListWidget->rowCount()-1, 2, ID);

        if(Common::Settings->value("matrixMode").toBool() == true && (name->text() == "Matrix" || name->text() == "The Matrix")) {
            name->setForeground(QBrush(QColor(0,150,0)));
        }

        numberOfParsedMovies++;
    }
    if(Common::Settings->value("moreLogs").toBool() == true)
        Common::Log->append(tr("Movies fetched from database: %1").arg(QString::number(numberOfParsedMovies)), eLog::Notice);

    /* QUICK FILTER PART*/

    QSqlQuery tagQuery;
    if(!tagQuery.exec("SELECT * FROM tags;"))
        Common::LogDatabaseError(&tagQuery);

    QString sFilter = m_ui->QuickSearchLineEdit->text();
    for(int row = 0 ; row < m_ui->MoviesListWidget->rowCount() ; row++)
    {
        bool bRowValid = false;
        // Check for quick search text
        for(int column = 0 ; column < m_ui->MoviesListWidget->columnCount()-1 ; column++)
        {
            QString sCell = m_ui->MoviesListWidget->item(row, column)->text();
            enum Qt::CaseSensitivity eCase = Common::Settings->value("quickSearchCaseSensitive").toBool() ? Qt::CaseSensitive : Qt::CaseInsensitive;

            if(sCell.contains(sFilter, eCase) == true)
            {
                bRowValid = true;
                break;
            }
        }

        // Check for selected tags
        if(bRowValid == true)
        {
            int nID = m_ui->MoviesListWidget->item(row, 2)->text().toInt();
            for(int nFilterTag = 0 ; nFilterTag < m_selectedTagsScrollArea->widget()->layout()->count()-1 ; nFilterTag++) {
                Tag* tag = (Tag*)m_selectedTagsScrollArea->widget()->layout()->itemAt(nFilterTag)->widget();
                bool bHasMovieFilterTag = false;
                tagQuery.first();
                tagQuery.previous();
                while(tagQuery.next()) {
                    if(tagQuery.value(0).toInt() == nID && QString::compare(tagQuery.value(1).toString(), tag->text()) == 0) {
                        bHasMovieFilterTag = true;
                        break;
                    }
                }
                if(bHasMovieFilterTag == false) {
                    bRowValid = false;
                    break;
                }
            }
        }

        if(bRowValid == true && m_ui->MoviesListWidget->isVisible() == false)
            m_ui->MoviesListWidget->showRow(row);
        else if(bRowValid == false && m_ui->MoviesListWidget->isVisible() == true)
            m_ui->MoviesListWidget->hideRow(row);
    }

    m_ui->MoviesListWidget->blockSignals(false);
    m_ui->MoviesListWidget->setSortingEnabled(true);
    m_ui->MoviesListWidget->sortItems(0);
    m_ui->DisplayedMovieCountLabel->setText(tr("Movies: %1").arg(QString::number(m_ui->MoviesListWidget->rowCount())));

    int nMovieID;

    if(m_ui->MoviesListWidget->rowCount() == 0)
        nMovieID = -1;
    else {
        if(getIndexOfMovie(nSavedSelectedMovieID) == -1)
           m_ui->MoviesListWidget->setCurrentCell(0, 0);
        else
            m_ui->MoviesListWidget->setCurrentCell(getIndexOfMovie(nSavedSelectedMovieID), 0);

        nMovieID = m_ui->MoviesListWidget->item(m_ui->MoviesListWidget->currentRow(),2)->text().toInt();
    }

    fillMovieInfos(nMovieID);
}

void MainWindow::fillMovieInfos(int nMovieID) {

    if(Common::Settings->value("moreLogs").toBool()) {
        qDebug() << "fillMovieInfos called with ID: " << nMovieID;
    }

    if(nMovieID == -1) {
        m_ui->MovieTitleLabel->setText(tr("Select a movie to see its informations"));
        m_ui->MovieTitleLabel->setAlignment(Qt::AlignHCenter);
        m_ui->FirstViewLabel->setText("");
        m_ui->LastViewLabel->setText("");
        m_ui->RatingLabel->setText("");
        m_ui->ViewsLabel->setText("");
        m_ui->PosterLabel->setPixmap(QPixmap());
        m_ui->RatingLabel->setPixmap(QPixmap());
        for(int i = m_movieTagsScrollArea->widget()->layout()->count()-1 ; i >= 0 ; i--) {
            delete m_movieTagsScrollArea->widget()->layout()->itemAt(i)->widget();
        }
        for(int i = m_ui->CustomInfosLayout->count()-1 ; i >= 0 ; i--) {
            delete m_ui->CustomInfosLayout->itemAt(i)->widget();
        }

        return;
    }

    QString ID = QString::number(nMovieID);

    //Fetch the number of views of the current movie
    QSqlQuery movieQuery;
    if(!movieQuery.exec("SELECT Name, Poster FROM movies WHERE ID='"+ID+"'"))
        Common::LogDatabaseError(&movieQuery);
    movieQuery.first();

    m_ui->MovieTitleLabel->setText(movieQuery.value(0).toString());
    Common::DisplayPoster(m_ui->PosterLabel, 400, 1, m_savepath + QDir::separator() + "Posters" + QDir::separator() + movieQuery.value(1).toString());

    //Fetch the number of views of the current movie
    QSqlQuery viewsQuery;
    if(!viewsQuery.exec("SELECT COUNT(*) FROM views WHERE ID_Movie='"+ID+"'"))
        Common::LogDatabaseError(&viewsQuery);
    viewsQuery.first();
    //
    m_ui->ViewsLabel->setText(tr("Viewed %1 time(s)").arg(viewsQuery.value(0).toInt()));


    //Fetch the first view of the current movie
    QSqlQuery firstViewQuery;
    if(!firstViewQuery.exec("SELECT ViewDate FROM views WHERE ID_Movie='"+ID+"' AND NOT ViewDate='?' ORDER BY ViewDate ASC LIMIT 1"))
        Common::LogDatabaseError(&firstViewQuery);
    firstViewQuery.first();
    if(firstViewQuery.value(0).toString()=="") {
        m_ui->FirstViewLabel->setText(tr("First view: ?"));
    }
    else {
        m_ui->FirstViewLabel->setText(tr("First view: %1").arg(firstViewQuery.value(0).toDate().toString(Common::Settings->value("dateFormat").toString())));
    }

    //Fetch the last view of the current movie
    QSqlQuery lastViewQuery;
    if(!lastViewQuery.exec("SELECT ViewDate FROM views WHERE ID_Movie='"+ID+"' AND NOT ViewDate='?' ORDER BY ViewDate DESC LIMIT 1"))
        Common::LogDatabaseError(&lastViewQuery);
    lastViewQuery.first();
    if(lastViewQuery.value(0).toString()=="") {
        m_ui->LastViewLabel->setText(tr("Last view: ?"));
    }
    else {
        m_ui->LastViewLabel->setText(tr("Last view: %1").arg(lastViewQuery.value(0).toDate().toString(Common::Settings->value("dateFormat").toString())));
    }

    QSqlQuery hasUnknownView;
    if(!hasUnknownView.exec("SELECT ViewDate FROM views WHERE ID_Movie='"+ID+"' AND ViewDate='?'"))
        Common::LogDatabaseError(&lastViewQuery);
    hasUnknownView.first();
    if(!hasUnknownView.isNull(0)) {
        m_ui->FirstViewLabel->setStyleSheet("color:red;");
    }
    else {
        m_ui->FirstViewLabel->setStyleSheet("");
    }

    QSqlQuery ratingQuery;
    if(!ratingQuery.exec("SELECT Rating FROM movies WHERE ID='"+ID+"'"))
        Common::LogDatabaseError(&ratingQuery);
    ratingQuery.first();
    Common::ratingToStar(ratingQuery.value(0).toInt(), m_ui->RatingLabel);

    QSqlQuery tagsQuery;
    if(!tagsQuery.exec("SELECT Tag FROM tags WHERE ID_Movie='"+ID+"'"))
        Common::LogDatabaseError(&tagsQuery);

    // Clear tags from layout
    for(int i = m_movieTagsScrollArea->widget()->layout()->count()-1 ; i >= 0 ; i--) {
        delete m_movieTagsScrollArea->widget()->layout()->itemAt(i)->widget();
    }

    QLayoutItem* spacer = m_movieTagsScrollArea->widget()->layout()->takeAt(m_movieTagsScrollArea->widget()->layout()->count()-1);
    while(tagsQuery.next()) {
        Tag* tag = new Tag(tagsQuery.value(0).toString());

        QObject::connect(tag, SIGNAL(clicked(Tag*)), this, SLOT(clickedTag(Tag*)));

        m_movieTagsScrollArea->widget()->layout()->addWidget(tag);
    }
    m_movieTagsScrollArea->widget()->layout()->addItem(spacer);

    // Add custom columns informations
    QSqlQuery customColumnsQuery;
    if(!customColumnsQuery.exec("SELECT Name FROM columns"))
            Common::LogDatabaseError(&customColumnsQuery);

    int nCustomColumnCount = 0;
    QString sCustomColumns;
    QStringList sCustomColumnsNameList;
    while(customColumnsQuery.next()) {
        sCustomColumns.append(" \"" + customColumnsQuery.value(0).toString() + "\",");
        nCustomColumnCount++;
        sCustomColumnsNameList << customColumnsQuery.value(0).toString();
    }
    sCustomColumns.removeLast(); // Removes the last ","
    if(sCustomColumns.isEmpty() == false) {
        QString sRequest = "SELECT" + sCustomColumns + " FROM movies WHERE ID='"+ID+"'";
        QSqlQuery customColumnsInformationsQuery;
        if(!customColumnsInformationsQuery.exec(sRequest))
            Common::LogDatabaseError(&customColumnsInformationsQuery);
        customColumnsInformationsQuery.first();

        // Clear custom columns informations from layout
        for(int i = m_ui->CustomInfosLayout->count()-1 ; i >= 0 ; i--) {
            delete m_ui->CustomInfosLayout->itemAt(i)->widget();
        }
        for(int nColumn = 0; nColumn < nCustomColumnCount; nColumn++)
        {
            if(customColumnsInformationsQuery.value(nColumn).toString().length() != 0)
            {
                QString sValue = customColumnsInformationsQuery.value(nColumn).toString();
                bool bIsIntger;
                int nValue = sValue.toInt(&bIsIntger);
                bool bIsDouble;
                double fValue = sValue.toDouble(&bIsDouble);
                if(bIsIntger == true)
                    sValue = m_locale->toString(nValue);
                else if(bIsDouble == true)
                    sValue = m_locale->toString(fValue);
                QLabel* label = new QLabel(QString(tr("%1: %2")).arg(sCustomColumnsNameList.at(nColumn), sValue));
                m_ui->CustomInfosLayout->addWidget(label);
            }
        }
    }
}

void MainWindow::removeUnusedTags() {
    QString removedTags;

    QSqlQuery tagsQuery;
    if(!tagsQuery.exec("SELECT Tag FROM tags"))
        Common::LogDatabaseError(&tagsQuery);

    QSqlQuery tagsInfoQuery;
    if(!tagsInfoQuery.exec("SELECT Tag FROM tagsInfo"))
        Common::LogDatabaseError(&tagsInfoQuery);

    bool found;

    //Foreach tag
    while(tagsInfoQuery.next()) {
        QSqlQuery deleteTagQuery;
        found = false;
        tagsQuery.first();
        tagsQuery.previous();
        //Foreach links between Tag and movie
        while(tagsQuery.next()) {
            //If tag exist in link table
            if(QString::compare(tagsQuery.value(0).toString(),tagsInfoQuery.value(0).toString()) == 0) {
                found = true;
                break;
            }
        }
        if(found == false) {
            if(!deleteTagQuery.exec("DELETE FROM tagsInfo WHERE Tag=\""+tagsInfoQuery.value(0).toString()+"\";")) {
                Common::LogDatabaseError(&deleteTagQuery);
            }
            removedTags.append(tagsInfoQuery.value(0).toString() + ", ");
        }
    }
    removedTags.remove(removedTags.length()-2, removedTags.length());
    if(removedTags.length() > 0) {
        Common::Log->append(tr("Following tags are no longer used, they're deleted: %1").arg(removedTags), eLog::Notice);
    }

}

void MainWindow::importDB() {
    QString file = QFileDialog::getOpenFileName(this, tr("Import"), QString(), tr("JSON (*.json)"));
    QFile jsonFile(file);
    //Test if the file is correctly opened
    if (!jsonFile.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Error"), jsonFile.errorString());
    }
    else {
        int answer = QMessageBox::question(this, tr("Import"), tr("This operation will remove all actual views, do you want to continue?"));
        if (answer == QMessageBox::Yes) {

            QSqlQuery deleteQuery;
            if(!deleteQuery.exec("DELETE FROM movies"))
                Common::LogDatabaseError(&deleteQuery);
            if(!deleteQuery.exec("DELETE FROM views"))
                Common::LogDatabaseError(&deleteQuery);
            if(!deleteQuery.exec("DELETE FROM tags"))
                Common::LogDatabaseError(&deleteQuery);
            if(!deleteQuery.exec("DELETE FROM tagsInfo"))
                Common::LogDatabaseError(&deleteQuery);

            QString val = jsonFile.readAll();
            jsonFile.close();
            QJsonObject main = QJsonDocument::fromJson(val.toUtf8()).object();
            foreach(const QString& mainKey, main.keys()) {
                if(mainKey == "movies") {
                    QJsonObject movies = main.value(mainKey).toObject();
                    foreach(const QString& movieKey, movies.keys()) {
                        QJsonObject movie = movies.value(movieKey).toObject();
                        QSqlQuery query;
                        query.prepare("INSERT INTO movies (ID, Name, ReleaseYear, Rating, Poster) VALUES (?,?,?,?,?);");
                        query.bindValue(0, movie["ID"].toInt());
                        query.bindValue(1, movie["Name"].toString());
                        query.bindValue(2, movie["ReleaseYear"].toInt());
                        query.bindValue(3, movie["Rating"].toInt());
                        query.bindValue(4, movie["Poster"].toString());

                        if(!query.exec()){
                            Common::LogDatabaseError(&query);
                        }
                    }
                }
                else if(mainKey == "views") {
                    QJsonObject views = main.value(mainKey).toObject();
                    foreach(const QString& viewKey, views.keys()) {
                        QJsonObject view = views.value(viewKey).toObject();
                        QSqlQuery query;
                        query.prepare("INSERT INTO views (ID, ID_Movie, ViewDate, ViewType) VALUES (?,?,?,?);");
                        query.bindValue(0, view["ID"].toInt());
                        query.bindValue(1, view["ID_Movie"].toInt());
                        query.bindValue(2, view["ViewDate"].toString());
                        query.bindValue(3, view["ViewType"].toString());

                        if(!query.exec()){
                            Common::LogDatabaseError(&query);
                        }
                    }
                }
                else if(mainKey == "tags") {
                    QJsonObject tags = main.value(mainKey).toObject();
                    foreach(const QString& tagKey, tags.keys()) {
                        QJsonObject tag = tags.value(tagKey).toObject();
                        QSqlQuery query;
                        query.prepare("INSERT INTO tags (ID_Movie, Tag) VALUES (?,?);");
                        query.bindValue(0, tag["ID_Movie"].toInt());
                        query.bindValue(1, tag["Tag"].toString());

                        if(!query.exec()){
                            Common::LogDatabaseError(&query);
                        }
                    }
                }
                else if(mainKey == "tagsInfo") {
                    QJsonObject tagsInfo = main.value(mainKey).toObject();
                    foreach(const QString& tagInfoKey, tagsInfo.keys()) {
                        QJsonObject tagInfo = tagsInfo.value(tagInfoKey).toObject();
                        QSqlQuery query;
                        query.prepare("INSERT INTO tagsInfo (Tag, Color) VALUES (?,?);");
                        query.bindValue(0, tagInfo["Tag"].toString());
                        query.bindValue(1, tagInfo["Color"].toString());

                        if(!query.exec()){
                            Common::LogDatabaseError(&query);
                        }
                    }
                }
            }
        }
    }
    fillTable();
    fillGlobalStats();
}

void MainWindow::exportDB() {
    int i;
    QString file = QFileDialog::getSaveFileName(this, tr("Export"), QString(), tr("JSON (*.json)"));  //Get the save link
    //Creates a QFile with the fetched path
    QFile jsonFile(file);
    //Test if the file is correctly opened
    if (!jsonFile.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, tr("Error"), jsonFile.errorString());
    }

    QJsonObject mainObject;

    //Writes movies to JSON
    QJsonObject moviesObject;
    QSqlQuery moviesQuery;
    if(!moviesQuery.exec("SELECT ID, Name, ReleaseYear, Rating, Poster FROM movies;"))
        Common::LogDatabaseError(&moviesQuery);
    i=0;
    while(moviesQuery.next()) {
        i++;

        QJsonObject movieObject;

        movieObject.insert("ID", QJsonValue::fromVariant(moviesQuery.value(0).toInt()));
        movieObject.insert("Name", QJsonValue::fromVariant(moviesQuery.value(1).toString()));
        movieObject.insert("ReleaseYear", QJsonValue::fromVariant(moviesQuery.value(2).toInt()));
        movieObject.insert("Rating", QJsonValue::fromVariant(moviesQuery.value(3).toInt()));
        movieObject.insert("Poster", QJsonValue::fromVariant(moviesQuery.value(4).toString()));

        moviesObject.insert("movie" + QString::fromStdString(std::to_string(i)), movieObject);
    }
    mainObject.insert("movies", moviesObject);

    //Writes views to JSON
    QJsonObject viewsObject;
    QSqlQuery viewsQuery;
    if(!viewsQuery.exec("SELECT ID, ID_Movie, ViewDate, ViewType FROM views;"))
        Common::LogDatabaseError(&viewsQuery);
    i=0;
    while(viewsQuery.next()) {
        i++;

        QJsonObject viewObject;

        viewObject.insert("ID", QJsonValue::fromVariant(viewsQuery.value(0).toInt()));
        viewObject.insert("ID_Movie", QJsonValue::fromVariant(viewsQuery.value(1).toInt()));
        viewObject.insert("ViewDate", QJsonValue::fromVariant(viewsQuery.value(2).toString()));
        viewObject.insert("ViewType", QJsonValue::fromVariant(viewsQuery.value(3).toString()));

        viewsObject.insert("view" + QString::fromStdString(std::to_string(i)), viewObject);
    }
    mainObject.insert("views", viewsObject);

    //Writes tagsInfo to JSON
    QJsonObject tagsInfoObject;
    QSqlQuery tagsInfoQuery;
    if(!tagsInfoQuery.exec("SELECT Tag, Color FROM tagsInfo;"))
        Common::LogDatabaseError(&tagsInfoQuery);
    i=0;
    while(tagsInfoQuery.next()) {
        i++;

        QJsonObject tagInfoObject;

        tagInfoObject.insert("Tag", QJsonValue::fromVariant(tagsInfoQuery.value(0).toString()));
        tagInfoObject.insert("Color", QJsonValue::fromVariant(tagsInfoQuery.value(1).toString()));

        tagsInfoObject.insert("tagInfo" + QString::fromStdString(std::to_string(i)), tagInfoObject);
    }
    mainObject.insert("tagsInfo", tagsInfoObject);

    //Writes tags to JSON
    QJsonObject tagsObject;
    QSqlQuery tagsQuery;
    if(!tagsQuery.exec("SELECT ID_Movie, Tag FROM tags;"))
        Common::LogDatabaseError(&tagsQuery);
    i=0;
    while(tagsQuery.next()) {
        i++;

        QJsonObject tagObject;

        tagObject.insert("ID_Movie", QJsonValue::fromVariant(tagsQuery.value(0).toInt()));
        tagObject.insert("Tag", QJsonValue::fromVariant(tagsQuery.value(1).toString()));

        tagsObject.insert("tag" + QString::fromStdString(std::to_string(i)), tagObject);
    }
    mainObject.insert("tags", tagsObject);

    jsonFile.write(QJsonDocument(mainObject).toJson(QJsonDocument::Indented));
    jsonFile.close();
}

void MainWindow::addView(int nMovieID) {
    AddViewDialog* window = new AddViewDialog(this, nMovieID);
    window->show();
    if(window->exec() == 1)
    {
        QString movieName;
        QString movieYear;
        int nNewMovieID = -1;

        // Step 1: Add the new movie to the movies table if it's new
        if(window->IsSearchedMovieAnExistingMovie() == false) {

            movieName = window->getName();
            movieYear = QString::number(window->getReleaseYear());

            QString posterPath = "";
            if(window->getPosterPath() != "") {
                QImage poster(window->getPosterPath());
                if(poster.height() * poster.width() > 2000000) {
                    Common::Log->append(tr("Selected poster is large (%1x%2). Latencies can be felt.").arg(QString::number(poster.width()), QString::number(poster.height())), eLog::Warning);
                }

                //Processing poster moving and renaming
                QString ext = window->getPosterPath().remove(0, window->getPosterPath().lastIndexOf(".")+1);
                QString GUID = QString::number(QRandomGenerator::global()->generate());
                if(QFile::copy(window->getPosterPath(), m_savepath + QDir::separator() + "Posters" + QDir::separator() + GUID + "." + ext) == false) {
                    Common::Log->append(tr("Error while copying poster,\nOriginal path: %1\nDestination path: %2/Posters/%3.%4").arg(window->getPosterPath(), m_savepath, GUID, ext), eLog::Error);
                }
                posterPath = GUID+"."+ext;
            }

            // Creation of the request
            QString sRequest = "INSERT INTO movies (Name, ReleaseYear, Rating, Poster";
            for(int nColumn = 0; nColumn < window->getCustomColumnCount(); nColumn++) {
                sRequest.append(", \"" + window->getCustomColumnInputAt(nColumn)->getLabel() + "\"");
            }
            sRequest.append(") VALUES (?,?,?,?");

            for(int nColumn = 0; nColumn < window->getCustomColumnCount(); nColumn++) {
                sRequest.append(",?");
            }
            sRequest.append(");");

            QSqlQuery insertIntoMoviesQuery;
            insertIntoMoviesQuery.prepare(sRequest);
            insertIntoMoviesQuery.bindValue(0, window->getName());
            insertIntoMoviesQuery.bindValue(1, window->getReleaseYear());
            insertIntoMoviesQuery.bindValue(2, window->getRating());
            insertIntoMoviesQuery.bindValue(3, posterPath);
            for(int nColumn = 0; nColumn < window->getCustomColumnCount(); nColumn++) {
                CustomColumnLineEdit* input = window->getCustomColumnInputAt(nColumn);
                insertIntoMoviesQuery.bindValue(4 + nColumn, input->text());
            }

            if(!insertIntoMoviesQuery.exec()){
                Common::Log->append(tr("Unable to insert the view (error 1)"), eLog::Error);
                Common::LogDatabaseError(&insertIntoMoviesQuery);
                return;
            }
            nNewMovieID = insertIntoMoviesQuery.lastInsertId().toInt();
        }
        else {
            movieName = window->GetSearchedMovieText().remove(window->GetSearchedMovieText().length()-7, window->GetSearchedMovieText().length());
            movieYear = window->GetSearchedMovieText().remove(0, window->GetSearchedMovieText().length()-4);
            QSqlQuery viewedMovieIDQuery;
            if(!viewedMovieIDQuery.exec("SELECT ID FROM movies WHERE Name=\""+movieName+"\" AND ReleaseYear=\""+movieYear+"\"")) {
                Common::Log->append(tr("Unable to insert the view (error 2)"), eLog::Error);
                Common::LogDatabaseError(&viewedMovieIDQuery);
                return;
            }
            viewedMovieIDQuery.first();
            nNewMovieID = viewedMovieIDQuery.value(0).toInt();
        }

        // Security
        if(nNewMovieID == -1)
        {
            Common::Log->append(tr("Unable to insert the view (error 3)"), eLog::Error);
            return;
        }

        // Step 2: Add the view to the views table
        QSqlQuery insertIntoViewsQuery;
        insertIntoViewsQuery.prepare("INSERT INTO views (ID_Movie, ViewDate, ViewType) VALUES (?,?,?);");
        insertIntoViewsQuery.bindValue(0, nNewMovieID);
        insertIntoViewsQuery.bindValue(1, window->isDateUnknown() ? "?" : window->getViewDate().toString("yyyy-MM-dd"));
        insertIntoViewsQuery.bindValue(2, (int)(window->isTypeUnknown() ? eViewType::Unknown : window->getViewType()));

        if(!insertIntoViewsQuery.exec()){
            Common::Log->append(tr("Unable to insert the view (error 4)"), eLog::Error);
            Common::LogDatabaseError(&insertIntoViewsQuery);
            return;
        }

        // Step 3 : Add tags
        for(int i=0 ; i<window->getTags()->size() ; i++) {
            QString hexcolor = "";

            // Generates a random color (unused for now)
            for(int j = 0 ; j < 6 ; j++) {
                hexcolor.append(QString::number(rand() % 9));
            }

            // If the tag isn't already existing
            if(window->GetTagsList().contains(window->getTags()->at(i)) == false)
            {
                QSqlQuery insertIntoTagsInfoQuery;

                insertIntoTagsInfoQuery.prepare("INSERT INTO tagsInfo (Tag, Color) VALUES (?,?);");
                insertIntoTagsInfoQuery.bindValue(0, window->getTags()->at(i));
                insertIntoTagsInfoQuery.bindValue(1, hexcolor);

                if(!insertIntoTagsInfoQuery.exec()){
                    Common::Log->append(tr("Unable to insert the view (error 5)"), eLog::Error);
                    Common::LogDatabaseError(&insertIntoTagsInfoQuery);
                }
            }

            QSqlQuery insertIntoTagsQuery;
            insertIntoTagsQuery.prepare("INSERT INTO tags (ID_Movie, Tag) VALUES (?,?);");
            insertIntoTagsQuery.bindValue(0, nNewMovieID);
            insertIntoTagsQuery.bindValue(1, window->getTags()->at(i));

            if(!insertIntoTagsQuery.exec()){
                Common::Log->append(tr("Unable to insert the view (error 6)"), eLog::Error);
                Common::LogDatabaseError(&insertIntoTagsQuery);
            }

        }

        // Step 4 : Update main window
        fillGlobalStats();
        fillTable();
        if(nMovieID == -1) {
            m_ui->MoviesListWidget->setCurrentCell(getIndexOfMovie(nNewMovieID), 0);
            fillMovieInfos(nNewMovieID);
        }
        else {
            m_ui->MoviesListWidget->setCurrentCell(getIndexOfMovie(nMovieID), 0);
            fillMovieInfos(nMovieID);
        }
    }
}

void MainWindow::editViews(int nMovieID) {

    if(Common::Settings->value("moreLogs").toBool())
        qDebug() << "Display views of the selected movie, ID: " << nMovieID;

    EditViewsDialog* window = new EditViewsDialog(&nMovieID, this);
    window->show();
    if(window->exec() == 1) {
        if (window->edited()) {
            if(window->GetViewsCount() == 0) {
                Common::Log->append(tr("Movie whose ID is %1 has no more views, it is removed").arg(nMovieID), eLog::Notice);
                this->deleteMovie(nMovieID);
            }
            fillTable();
            fillGlobalStats();
        }
    }
}

void MainWindow::editMovie(int nMovieID) {

    if(Common::Settings->value("moreLogs").toBool())
        qDebug() << "Movie's ID : " << nMovieID;

    EditMovieDialog* window = new EditMovieDialog(QString::number(nMovieID), this);
    window->show();
    if(window->exec() == 1) {

        QString ID = QString::number(nMovieID);
        QString GUID = "";
        QString ext = "";

        QSqlQuery existingMoviesQuery;
        if(!existingMoviesQuery.exec("SELECT Name, ReleaseYear, ID, Poster FROM movies")) {
            Common::LogDatabaseError(&existingMoviesQuery);
            return;
        }
        while(existingMoviesQuery.next()) {
            if(QString::compare(window->getMovieName(), existingMoviesQuery.value(0).toString()) == 0
            && QString::compare(window->getReleaseYear(), existingMoviesQuery.value(1).toString()) == 0
            && nMovieID != existingMoviesQuery.value(2).toInt()) {
                int answer = QMessageBox::question(this, tr("Movie already exists"), tr("There is already a movie with this name and release year, views will be combined, do you want to continue?"));
                if(answer == QMessageBox::Yes) {
                    Common::Log->append(tr("Merging of the following movies' ID: %1 and %2").arg(existingMoviesQuery.value(2).toString(), ID) , eLog::Notice);
                    QSqlQuery changeViewsIDQuery;
                    if(!changeViewsIDQuery.exec("UPDATE views SET ID_Movie=\""+existingMoviesQuery.value(2).toString()+"\" WHERE ID_Movie=\""+ID+"\"")) {
                        Common::LogDatabaseError(&changeViewsIDQuery);
                        return;
                    }

                    QSqlQuery MoviePosterToDeleteQuery;
                    if(!MoviePosterToDeleteQuery.exec("SELECT Poster FROM Movies WHERE ID=\""+ID+"\""))
                        Common::LogDatabaseError(&MoviePosterToDeleteQuery);

                    MoviePosterToDeleteQuery.first();
                    QFile::remove(m_savepath + QDir::separator() + "Posters" + QDir::separator() + MoviePosterToDeleteQuery.value(0).toString());

                    QSqlQuery deleteMovieQuery;
                    if(!deleteMovieQuery.exec("DELETE FROM movies WHERE ID=\""+ID+"\""))
                        Common::LogDatabaseError(&deleteMovieQuery);
                    QSqlQuery deleteMovieTags;
                    if(!deleteMovieTags.exec("DELETE FROM tags WHERE ID_Movie=\""+ID+"\""))
                        Common::LogDatabaseError(&deleteMovieTags);

                    removeUnusedTags();
                    resetFilters();
                    m_ui->MoviesListWidget->setCurrentCell(getIndexOfMovie(existingMoviesQuery.value(2).toInt()), 0);
                    fillMovieInfos(existingMoviesQuery.value(2).toInt());
                    fillGlobalStats();
                }
                return;
            }
        }

        QSqlQuery editMovieQuery;

        QString sUpdateMovieRequest = "UPDATE movies SET Name=\""+window->getMovieName()+"\", ReleaseYear=\""+window->getReleaseYear()+
                              "\", Rating=\""+QString::number(window->getRating())+"\" ";
        for(int nColumn = 0; nColumn < window->getCustomColumnCount(); nColumn++) {
            QString sText;
            if(window->getCustomColumnInputAt(nColumn) != nullptr)
            {
                sText = window->getCustomColumnInputAt(nColumn)->text();
                sUpdateMovieRequest.append(", \"" + window->getCustomColumnInputAt(nColumn)->getLabel() + "\"=\"" + sText + "\"");

            }
        }

        if(window->newPoster()) {
            // Delete old poster
            QSqlQuery posterQuery;
            if(!posterQuery.exec("SELECT Poster FROM movies WHERE ID=\""+ID+"\";"))
                Common::LogDatabaseError(&posterQuery);
            posterQuery.first();
            QFile::remove(m_savepath+"\\Posters\\"+posterQuery.value(0).toString());

            ext = window->getPosterPath().remove(0, window->getPosterPath().lastIndexOf(".")+1);

            GUID = QString::number(QRandomGenerator::global()->generate());
            if(QFile::copy(window->getPosterPath(), m_savepath + QDir::separator() + "Posters" + QDir::separator() + GUID + "." + ext) == false) {
                Common::Log->append(tr("Error while copying poster,\nOriginal path: %1\nDestination path: %2/Posters/%3.%4").arg(window->getPosterPath(), m_savepath, GUID, ext), eLog::Error);
            }

            // Add poster ID modification to the update request
            sUpdateMovieRequest += ", Poster=\""+GUID+"."+ext+"\"";
        }

        sUpdateMovieRequest.append(" WHERE ID=\""+ID+"\";");

        if(!editMovieQuery.exec(sUpdateMovieRequest)) {
            Common::LogDatabaseError(&editMovieQuery);
        }


        QSqlQuery removeMovieTagsQuery;
        if(!removeMovieTagsQuery.exec("DELETE FROM tags WHERE ID_Movie="+ID)){
            Common::LogDatabaseError(&removeMovieTagsQuery);
        }



        QSqlQuery TagsInfoQuery;
        if(!TagsInfoQuery.exec("SELECT Tag FROM TagsInfo"))
            Common::LogDatabaseError(&TagsInfoQuery);
        bool tagAlreadyExist;

        for(int i=0 ; i<window->getTags()->size() ; i++) {
            tagAlreadyExist = false;

            QString hexcolor = "";
            for(int j = 0 ; j<6 ; j++) {
                hexcolor.append(QString::number(rand() % 9));
            }

            QSqlQuery insertIntoTagsInfoQuery;

            TagsInfoQuery.first();
            TagsInfoQuery.previous();
            while(TagsInfoQuery.next()) {
                if(QString::compare(window->getTags()->at(i), TagsInfoQuery.value(0).toString()) == 0) {
                    tagAlreadyExist = true;
                    break;
                }
            }
            if(!tagAlreadyExist) {
                insertIntoTagsInfoQuery.prepare("INSERT INTO tagsInfo (Tag, Color) VALUES (?,?);");
                insertIntoTagsInfoQuery.bindValue(0, window->getTags()->at(i));
                insertIntoTagsInfoQuery.bindValue(1, hexcolor);

                if(!insertIntoTagsInfoQuery.exec()){
                    Common::LogDatabaseError(&insertIntoTagsInfoQuery);
                }
            }
            QSqlQuery insertIntoTagsQuery;

            insertIntoTagsQuery.prepare("INSERT INTO tags (ID_Movie, Tag) VALUES (?,?);");
            insertIntoTagsQuery.bindValue(0, ID);
            insertIntoTagsQuery.bindValue(1, window->getTags()->at(i));

            if(!insertIntoTagsQuery.exec()){
                Common::LogDatabaseError(&insertIntoTagsQuery);
            }

        }

        removeUnusedTags();
        fillTable();
        fillMovieInfos(nMovieID);

    }
}

void MainWindow::deleteMovieConfirmation(int nMovieID) {
    QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Remove the movie"), tr("Are you sure do you want to remove the movie? Its views will be removed."));
    if(reply == QMessageBox::Yes)
        deleteMovie(nMovieID);
}

void MainWindow::deleteMovie(int nMovieID) {
    int savedRow = 0;
    QSqlQuery deleteMovieQuery;
    QSqlQuery deleteAssociatedViewsQuery;
    QSqlQuery deleteAssociatedTagsQuery;
    QSqlQuery posterQuery;

    QString ID = QString::number(nMovieID);
    savedRow = m_ui->MoviesListWidget->currentRow();

    if(!posterQuery.exec("SELECT Poster FROM movies WHERE ID=\""+ID+"\";"))
        Common::LogDatabaseError(&posterQuery);

    posterQuery.first();
    QFile::remove(m_savepath + QDir::separator() + "Posters" + QDir::separator() + posterQuery.value(0).toString());


    if(!deleteMovieQuery.exec("DELETE FROM movies WHERE ID=\""+ID+"\";")) {
        Common::LogDatabaseError(&deleteMovieQuery);
    }

    if(!deleteAssociatedViewsQuery.exec("DELETE FROM views WHERE ID_Movie=\""+ID+"\";")) {
        Common::LogDatabaseError(&deleteAssociatedViewsQuery);
    }

    if(!deleteAssociatedTagsQuery.exec("DELETE FROM tags WHERE ID_Movie=\""+ID+"\";")) {
        Common::LogDatabaseError(&deleteAssociatedTagsQuery);
    }

    removeUnusedTags();
    resetFilters();
    if(savedRow == m_ui->MoviesListWidget->rowCount())
        savedRow--;
    if(savedRow == -1)
        savedRow = 0;
    m_ui->MoviesListWidget->setCurrentCell(savedRow, 0);

    if(m_ui->MoviesListWidget->rowCount() == 0)
        fillMovieInfos(-1);
    else
        fillMovieInfos(m_ui->MoviesListWidget->item(m_ui->MoviesListWidget->currentRow(),2)->text().toInt());
    fillGlobalStats();
}

void MainWindow::openFilters() {
    FiltersDialog* window = new FiltersDialog(m_customColumnsRequestFilter);
    window->show();
    if(window->exec() == 1) {
        m_ui->ResetFiltersButton->setEnabled(true);

        m_customColumnsRequestFilter = window->FiltersToSQLRequest();

        delete window;

        fillTable();
    }
}

void MainWindow::openLog() {
    if(LogDialog::instancesCount() == 0) {
        LogDialog* window = new LogDialog(this);
        window->show();
        if(window->exec() == 0) {
            delete window;
        }
    }
    else {
        Common::Log->append(tr("Log already open"), eLog::Warning);
    }
}

void MainWindow::openAbout() {
    if(AboutDialog::instancesCount() == 0) {
        AboutDialog* window = new AboutDialog(this);
        window->show();
        if(window->exec() == 0) {
            delete window;
        }
    }
    else {
        Common::Log->append(tr("About window already open"), eLog::Warning);
    }
}

void MainWindow::on_whatsnewAct_triggered() {
    if(ChangelogDialog::instancesCount() == 0) {
        ChangelogDialog* window = new ChangelogDialog(this);
        window->show();
        if(window->exec() == 0) {
            delete window;
        }
    }
    else {
        Common::Log->append(tr("News window already open"), eLog::Warning);
    }
}

void MainWindow::openSettings() {
    OptionsDialog* window = new OptionsDialog(this);
    window->show();
    if(window->exec() == 1) {
        delete window;
        refreshLanguage();
        refreshTheme();
        fillTable();

        // Update movie's table columns
        QStringList sExistingColumns;
        QStringList sCustomColumns;
        QStringList sCustomColumnsType;

        QSqlQuery columnsQuery;
        if(!columnsQuery.exec("SELECT Name, Type, Min, Max, Precision, TextMaxLength FROM columns"))
            Common::LogDatabaseError(&columnsQuery);
        while(columnsQuery.next()) {
            sCustomColumns << columnsQuery.value(0).toString();
            sCustomColumnsType << columnsQuery.value(1).toString();
        }

        QSqlQuery existingColumnsQuery;
        if(!existingColumnsQuery.exec("PRAGMA table_info(movies)"))
            Common::LogDatabaseError(&existingColumnsQuery);
        while(existingColumnsQuery.next())
            sExistingColumns << existingColumnsQuery.value(1).toString();

        // Remove dropped columns
        existingColumnsQuery.first();
        existingColumnsQuery.previous();
        bool bDeleteColumn;
        while(existingColumnsQuery.next()) {
            bDeleteColumn = false;
            QString sColumn = existingColumnsQuery.value(1).toString();

            // No check for mandatory columns
            if(QString::compare(sColumn, "ID") == 0
            || QString::compare(sColumn, "Name") == 0
            || QString::compare(sColumn, "ReleaseYear") == 0
            || QString::compare(sColumn, "Rating") == 0
            || QString::compare(sColumn, "Poster") == 0)
                continue;

            // If column is removed or renamed, it is dropped from movies table,
            // Else, if type changed, the column is updated (and data of the column is deleted)
            if(sCustomColumns.contains(sColumn) == false) {
                bDeleteColumn = true;
            }
            else {
                QString sType = existingColumnsQuery.value(2).toString();
                int nIndexOfColumn = sCustomColumns.indexOf(sColumn);
                int nType = 0;

                if(QString::compare(sType, "INTEGER") == 0)
                    nType = 0;
                else if(QString::compare(sType, "REAL") == 0)
                    nType = 1;
                else if(QString::compare(sType, "TEXT") == 0)
                    nType = 2;

                if(nType != sCustomColumnsType.at(nIndexOfColumn).toInt()) {
                    bDeleteColumn = true;
                }
            }

            if(bDeleteColumn == true) {
                QSqlQuery dropColumnFromTable;
                QString sRequest = "ALTER TABLE movies DROP COLUMN \"" + sColumn + "\"";
                if(!dropColumnFromTable.exec(sRequest))
                    Common::LogDatabaseError(&dropColumnFromTable);
                else {
                    Common::Log->append(tr("Column %1 successfully dropped from movies table").arg(sColumn), eLog::Success);
                    sExistingColumns.removeAt(sExistingColumns.indexOf(sColumn));
                }
            }
        }

        // Add columns
        columnsQuery.first();
        columnsQuery.previous();
        while(columnsQuery.next()) {
            if(sExistingColumns.contains(columnsQuery.value(0).toString()) == false) {
                QSqlQuery addColumnToTable;
                QString sRequest = "ALTER TABLE movies ADD COLUMN \"" + columnsQuery.value(0).toString() + "\" ";
                switch(columnsQuery.value(1).toInt()) {
                case 0:
                    sRequest.append("INTEGER");
                    break;
                case 1:
                    sRequest.append("REAL");
                    break;
                case 2:
                    sRequest.append("TEXT");
                    break;
                default:
                    Common::Log->append(tr("Unknown column type, can't add column %1 to movies table").arg(columnsQuery.value(0).toString()), eLog::Error);
                    break;
                }

                if(!addColumnToTable.exec(sRequest))
                    Common::LogDatabaseError(&addColumnToTable);
                else
                    Common::Log->append(tr("Column %1 successfully added to movies table").arg(columnsQuery.value(0).toString()), eLog::Success);
            }
        }

    }
    m_ui->DisplayedMovieCountLabel->setText(tr("Movies: %1").arg(QString::number(m_ui->MoviesListWidget->rowCount())));
}

void MainWindow::resetFilters() {
    m_ui->ResetFiltersButton->setEnabled(false);
    m_customColumnsRequestFilter = "";
    fillTable();
}

void MainWindow::customMenuRequested(QPoint pos) {
    if(m_ui->MoviesListWidget->currentRow() == -1)
        return;

    int nMovieID = m_ui->MoviesListWidget->item(m_ui->MoviesListWidget->currentRow(),2)->text().toInt();
    fillMovieInfos(nMovieID);

    enum eTheme eTheme = (enum eTheme)Common::Settings->value("theme").toInt();

    QMenu *menu = new QMenu(this);

    QAction* addViewAction = new QAction(tr("Add a view"), this);
    addViewAction->setIcon(Common::GetIconAccordingToTheme(eTheme, "plus.png"));

    QAction* editAction = new QAction(tr("Edit"), this);
    editAction->setIcon(Common::GetIconAccordingToTheme(eTheme, "edit.png"));

    QAction* deleteAction = new QAction(tr("Remove"), this);
    deleteAction->setIcon(Common::GetIconAccordingToTheme(eTheme, "delete.png"));


    menu->addAction(addViewAction);
    menu->addAction(editAction);
    menu->addAction(deleteAction);

    QSignalMapper* addViewSignalMapper = new QSignalMapper();
    QObject::connect(addViewSignalMapper, SIGNAL(mappedInt(int)), this, SLOT(addView(int)));
    addViewSignalMapper->setMapping(addViewAction, nMovieID);
    QObject::connect(addViewAction, SIGNAL(triggered()), addViewSignalMapper, SLOT(map()));

    QSignalMapper* editMovieSignalMapper = new QSignalMapper();
    QObject::connect(editMovieSignalMapper, SIGNAL(mappedInt(int)), this, SLOT(editMovie(int)));
    editMovieSignalMapper->setMapping(editAction, nMovieID);
    QObject::connect(editAction, SIGNAL(triggered()), editMovieSignalMapper, SLOT(map()));

    QSignalMapper* deleteMovieSignalMapper = new QSignalMapper();
    QObject::connect(deleteMovieSignalMapper, SIGNAL(mappedInt(int)), this, SLOT(deleteMovieConfirmation(int)));
    deleteMovieSignalMapper->setMapping(deleteAction, nMovieID);
    QObject::connect(deleteAction, SIGNAL(triggered()), deleteMovieSignalMapper, SLOT(map()));


    menu->popup(m_ui->MoviesListWidget->viewport()->mapToGlobal(pos));
}

void MainWindow::menuBarConnectors() {
    QObject::connect(m_ui->QuitAct, SIGNAL(triggered()), m_app, SLOT(quit()));
    QObject::connect(m_ui->LogAct, SIGNAL(triggered()), this, SLOT(openLog()));
    QObject::connect(m_ui->AboutAct, SIGNAL(triggered()), this, SLOT(openAbout()));
    QObject::connect(m_ui->OptionsAct, SIGNAL(triggered()), this, SLOT(openSettings()));
    QObject::connect(m_ui->ImportAct, SIGNAL(triggered()), this, SLOT(importDB()));
    QObject::connect(m_ui->ExportAct, SIGNAL(triggered()), this, SLOT(exportDB()));
    QObject::connect(m_ui->ChartAct, SIGNAL(triggered()), this, SLOT(openCharts()));
    QObject::connect(m_ui->CalendarAct, SIGNAL(triggered()), this, SLOT(openCalendar()));
    QObject::connect(m_ui->CheckForUpdateAct, SIGNAL(triggered()), this, SLOT(CheckForUpdates()));
    QObject::connect(m_ui->PayPalAct, SIGNAL(triggered()), this, SLOT(openPayPal()));
    QObject::connect(m_ui->MemoriesAct, SIGNAL(triggered()), this, SLOT(openMemories()));
}

void MainWindow::on_EasterEggAct_triggered() {
    qDebug() << "Why, Mr. Anderson, why? Why, why do you do it? Why, why opening a terminal? Why looking for this easter egg? Do you believe you’re searching for "
                   "something, for more than your curiosity? Can you tell me what it is, do you even know? Is it freedom or truth, perhaps "
                   "peace – could it be for love? Illusions, Mr. Anderson, vagaries of perception. Temporary constructs of a feeble human "
                   "intellect trying desperately to justify an existence that is without meaning or purpose. And all of them as artificial "
                   "as the MovieManager itself. Although, only a human mind could do something as insipid as lauching this through a terminal. "
                   "You must be able to see it, Mr. Anderson, you must know it by now! You can’t win, it’s pointless to keep searching! Why, Mr. Anderson, why, "
                   "why do you persist?";
}

void MainWindow::setMatrixMode(bool state) {

    QString cellText;

    for(int i = 0 ; i < m_ui->MoviesListWidget->rowCount() ; i++) {
        cellText = m_ui->MoviesListWidget->item(i,0)->text();
        if(cellText == "Matrix" || cellText == "The Matrix") {
            if(state == true) {
                m_ui->MoviesListWidget->item(i,0)->setForeground(QBrush(QColor(0,150,0)));
            }
            else {
                QTableWidgetItem* name = new QTableWidgetItem(cellText);
                m_ui->MoviesListWidget->removeCellWidget(i,0);
                m_ui->MoviesListWidget->setItem(i, 0, name);
            }
            break;
        }
    }
}

void MainWindow::refreshLanguage() {
    bool successLoad = false;
    QString path;

    switch((enum eLanguage)Common::Settings->value("language").toInt()) {
        case eLanguage::English :
            path = ":/localisations/Localisation/MovieManager_en_US.qm";
            m_locale = new QLocale(QLocale::English);
            break;
        case eLanguage::French :
            path = ":/localisations/Localisation/MovieManager_fr_FR.qm";
            m_locale = new QLocale(QLocale::French);
            break;
    }

    if(m_translator.load(path)) {
        m_app->installTranslator(&m_translator);
        successLoad = true;
    }

    if(!successLoad) {
        Common::Log->append(tr("Unable to load translation"), eLog::Error);
    }
    m_ui->retranslateUi(this);
    fillGlobalStats();
}

void MainWindow::refreshTheme() {
    QString path;
    enum eTheme eTheme = (enum eTheme)Common::Settings->value("theme").toInt();

    switch(eTheme) {
        case eTheme::System:
            if(qApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark)
                path = ":/styles/Styles/dark.qss";
            else
                path = ":/styles/Styles/classic.qss";
            break;
        case eTheme::Classic:
            path = ":/styles/Styles/classic.qss";
            break;
        case eTheme::Dark:
            path = ":/styles/Styles/dark.qss";
            break;
        case eTheme::MidnightPurple:
            path = ":/styles/Styles/midnightPurple.qss";
            break;
        case eTheme::OLED:
            path = ":/styles/Styles/oled.qss";
            break;
    }

    QFile qss(path);
    qss.open(QFile::ReadOnly);
    qApp->setStyleSheet(qss.readAll());
    qss.close();

    // Pride month
    if(QDate::currentDate().month() == 6)
    {
        m_ui->menubar->setStyleSheet("border-bottom: 3px solid qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0,"
                                     "stop: 0.125 #000000,"
                                     "stop: 0.125001 #784f17,stop: 0.25 #784f17,"
                                     "stop: 0.25001 #fe0000,stop: 0.375 #fe0000,"
                                     "stop: 0.375001 #ff8e01, stop: 0.5 #ff8e01,"
                                     "stop: 0.5001 #ffee00, stop: 0.625 #ffee00,"
                                     "stop: 0.625001 #028215, stop: 0.75 #028215,"
                                     "stop: 0.75001 #014bff, stop: 0.875 #014bff,"
                                     "stop: 0.875001 #8a018c, stop: 1 #8a018c);");
    }

    m_ui->ExportAct->setIcon(Common::GetIconAccordingToTheme(eTheme, "export.png"));
    m_ui->ImportAct->setIcon(Common::GetIconAccordingToTheme(eTheme, "import.png"));
    m_ui->QuitAct->setIcon(Common::GetIconAccordingToTheme(eTheme, "exit.png"));
    m_ui->OptionsAct->setIcon(Common::GetIconAccordingToTheme(eTheme, "settings.png"));
    m_ui->LogAct->setIcon(Common::GetIconAccordingToTheme(eTheme, "log.png"));
    m_ui->ChartAct->setIcon(Common::GetIconAccordingToTheme(eTheme, "chart.png"));
    m_ui->CalendarAct->setIcon(Common::GetIconAccordingToTheme(eTheme, "calendar.png"));
    m_ui->whatsnewAct->setIcon(Common::GetIconAccordingToTheme(eTheme, "github.png"));
    m_ui->AboutAct->setIcon(Common::GetIconAccordingToTheme(eTheme, "info.png"));
    m_ui->CheckForUpdateAct->setIcon(Common::GetIconAccordingToTheme(eTheme, "download.png"));
    m_ui->PayPalAct->setIcon(Common::GetIconAccordingToTheme(eTheme, "paypal.png"));
    m_ui->MemoriesAct->setIcon(Common::GetIconAccordingToTheme(eTheme, "flashback.png"));
}

void MainWindow::fillGlobalStats() {

    QSqlQuery uniqueViewQuery;
    if(!uniqueViewQuery.exec("SELECT COUNT(*) FROM movies;"))
            Common::LogDatabaseError(&uniqueViewQuery);
    uniqueViewQuery.first();

    QSqlQuery totalViewQuery;
    if(!totalViewQuery.exec("SELECT COUNT(*) FROM views;"))
            Common::LogDatabaseError(&totalViewQuery);
    totalViewQuery.first();

    QSqlQuery avgMovieYearQuery;
    if(!avgMovieYearQuery.exec("SELECT ReleaseYear, Rating FROM movies"))
            Common::LogDatabaseError(&avgMovieYearQuery);
    float avgMovieYear = 0;
    float avgRating = 0;
    float i=0;
    while(avgMovieYearQuery.next()) {
        avgMovieYear += (float)avgMovieYearQuery.value(0).toInt();
        avgRating += (float)avgMovieYearQuery.value(1).toInt();
        i++;
    }

    avgMovieYear /= i;
    avgRating /= i;



    float avgViews = totalViewQuery.value(0).toFloat()/uniqueViewQuery.value(0).toFloat();
    avgViews = round(avgViews*100)/100;
    avgRating = round(avgRating*100)/100;

    QSqlQuery newThisYearQuery;
    if(!newThisYearQuery.exec("SELECT count(*) FROM movies WHERE ID IN"
                          "(SELECT ID_Movie FROM views WHERE ID_Movie IN"
                          "(SELECT ID_Movie FROM views WHERE ViewDate BETWEEN '"+QString::number(QDate::currentDate().year())+"-01-01' AND '"+QString::number(QDate::currentDate().year())+"-12-31') AND ID_Movie NOT IN"
                          "(SELECT ID_Movie FROM views WHERE ViewDate < '"+QString::number(QDate::currentDate().year())+"-01-01' AND ViewDate != '?'))"))
        Common::LogDatabaseError(&newThisYearQuery);
    newThisYearQuery.first();

    QSqlQuery movieThisYearQuery;
    if(!movieThisYearQuery.exec("SELECT count(*) FROM views "
                                "WHERE ViewDate BETWEEN '"+QString::number(QDate::currentDate().year())+"-01-01' AND '"+QString::number(QDate::currentDate().year())+"-12-31'"))
        Common::LogDatabaseError(&movieThisYearQuery);
    movieThisYearQuery.first();

    m_ui->NewThisYearLabel->setText(tr("Discovered this year: %1").arg(newThisYearQuery.value(0).toString()));
    m_ui->TotalViewLabel->setText(tr("Views count: %1").arg(totalViewQuery.value(0).toString()));
    m_ui->AverageViewLabel->setText(tr("Average views: %1").arg(QString::number(avgViews)));
    m_ui->AverageYearLabel->setText(tr("Average viewed movie's year: %1").arg(QString::number(avgMovieYear)));
    m_ui->ViewThisYear->setText(tr("Viewed this year: %1").arg(movieThisYearQuery.value(0).toString()));
    m_ui->AverageRatingLabel->setText(tr("Average rating: %1").arg(QString::number(avgRating)));
}

void MainWindow::openCharts() {
    if(ChartsDialog::instancesCount() == 0) {
        ChartsDialog* window = new ChartsDialog(this);
        window->show();
        if(window->exec() == 0) {
            delete window;
        }
    }
    else {
        Common::Log->append(tr("Charts window already open"), eLog::Warning);
    }
}

void MainWindow::openCalendar() {
    if(CalendarDialog::instancesCount() == 0) {
        CalendarDialog* window = new CalendarDialog(this);
        window->show();
        if(window->exec() == 0) {
            delete window;
        }
    }
    else {
        Common::Log->append(tr("Calendar already open"), eLog::Warning);
    }
}

int MainWindow::getIndexOfMovie(int ID) {
    int row;
    for(row = 0 ; row < m_ui->MoviesListWidget->rowCount() ; row++) {
        if(ID == m_ui->MoviesListWidget->item(row,2)->text().toInt()) {
            return row;
        }
    }
    return -1;
}

void MainWindow::clickedTag(Tag* tag) {

    // If layout was empty, height is re-adjusted to fit tags (height forced to 0 if no tags selected)
    if(m_selectedTagsScrollArea->widget()->layout()->count() == 1)
        m_selectedTagsScrollArea->setMaximumHeight(45);


    bool isTagInLayout = false;
    for(int i = 0 ; i < m_selectedTagsScrollArea->widget()->layout()->count() - 1 ; i++) {
        Tag* t = (Tag*)m_selectedTagsScrollArea->widget()->layout()->itemAt(i)->widget();
        if(QString::compare(t->text(), tag->text()) == 0) {
            isTagInLayout = true;
            break;
        }
    }

    if(!isTagInLayout) {
        Tag* copiedTag = new Tag(tag);

        QObject::connect(copiedTag, SIGNAL(clicked(Tag*)), this, SLOT(clickedFilterTag(Tag*)));
        QObject::connect(copiedTag, SIGNAL(mouseEnter(Tag*)), this, SLOT(mouseEnteredTag(Tag*)));
        QObject::connect(copiedTag, SIGNAL(mouseLeave(Tag*)), this, SLOT(mouseLeftTag(Tag*)));

        QLayoutItem* spacer = m_selectedTagsScrollArea->widget()->layout()->takeAt(m_selectedTagsScrollArea->widget()->layout()->count()-1);
        m_selectedTagsScrollArea->widget()->layout()->addWidget(copiedTag);
        m_selectedTagsScrollArea->widget()->layout()->addItem(spacer);
        fillTable();
    }
}

void MainWindow::mouseEnteredTag(Tag* tag) {
    tag->setSavedTag(tag->text());
    tag->setText("❌");
}

void MainWindow::mouseLeftTag(Tag* tag) {
    tag->setText(tag->getSavedTag());
}

void MainWindow::clickedFilterTag(Tag* tag) {
    delete tag;
    // Widget height forced to 0 to avoid a vertical space if no tags are selected
    if(m_selectedTagsScrollArea->widget()->layout()->count() == 1)
        m_selectedTagsScrollArea->setMaximumHeight(0);
    fillTable();
}

void MainWindow::on_MoviesListWidget_cellClicked(int row, [[maybe_unused]] int column) {
    fillMovieInfos(m_ui->MoviesListWidget->item(row,2)->text().toInt());
}

void MainWindow::on_MoviesListWidget_cellDoubleClicked(int row, [[maybe_unused]] int column) {
    editViews(m_ui->MoviesListWidget->item(row,2)->text().toInt());
}

void MainWindow::on_ManageMovieViewsButton_clicked() {
    editViews(m_ui->MoviesListWidget->item(m_ui->MoviesListWidget->currentRow(),2)->text().toInt());
}

void MainWindow::CheckForUpdates(bool bManualTrigger)
{
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QNetworkReply* reply = manager->get(QNetworkRequest(QUrl("https://api.github.com/repos/AmbreM71/MovieManager/releases/latest")));

    connect(reply, &QNetworkReply::finished, [=]()
    {
        if(reply->error() == QNetworkReply::NoError)
        {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QString sLatestVersion = doc.object().value("tag_name").toString();
            QString sCurrentVersion = Common::getVersion();
            if(QString::compare(sCurrentVersion, sLatestVersion) != 0)
            {
                QMessageBox messageBox;
                messageBox.setWindowTitle("Check for updates");
                messageBox.setWindowIcon(Common::GetIconAccordingToColorScheme(qApp->styleHints()->colorScheme(), "download.png"));
                messageBox.setText(tr("A new version is available!\nLatest: %1\nCurrent: %2\n\nGo to the download page?").arg(sLatestVersion, sCurrentVersion));
                messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                messageBox.setDefaultButton(QMessageBox::Yes);
                messageBox.setIcon(QMessageBox::Question);
                if(messageBox.exec() == QMessageBox::Yes)
                {
                    QDesktopServices::openUrl(QUrl("https://github.com/AmbreM71/MovieManager/releases/latest", QUrl::TolerantMode));
                }
            }
            else
            {
                if(bManualTrigger == true)
                {
                    QMessageBox messageBox;
                    messageBox.setWindowTitle("Check for updates");
                    messageBox.setWindowIcon(Common::GetIconAccordingToColorScheme(qApp->styleHints()->colorScheme(), "download.png"));
                    messageBox.setText(tr("MovieManager is up to date"));
                    messageBox.setStandardButtons(QMessageBox::Ok);
                    messageBox.setDefaultButton(QMessageBox::Ok);
                    messageBox.setIcon(QMessageBox::Information);
                    messageBox.exec();
                }
            }
        }
        else
        {
            Common::Log->append(tr("Unable to fetch last version: %1").arg(reply->errorString()), eLog::Error);
        }
    });
}

QString MainWindow::getDatabaseVersion()
{
    QString sDatabaseVersion;
    QSqlQuery getDatabaseVersion;
    if(!getDatabaseVersion.exec("SELECT Version FROM Version"))
        Common::LogDatabaseError(&getDatabaseVersion);

    getDatabaseVersion.first();

    sDatabaseVersion = getDatabaseVersion.value(0).toString();

    return sDatabaseVersion;
}

void MainWindow::CheckDatabaseVersion()
{
    QString sDatabaseVersion = getDatabaseVersion();

    if(sDatabaseVersion == "1.2.0")
    {
        QSqlQuery query;
        if(!query.exec("ALTER TABLE Columns ADD COLUMN Optional INTEGER;"))
            Common::LogDatabaseError(&query);

        if(!query.exec("DELETE FROM version;"))
            Common::LogDatabaseError(&query);
        if(!query.exec("INSERT INTO Version (Version) VALUES (\"1.2.1\");"))
            Common::LogDatabaseError(&query);
        sDatabaseVersion = "1.2.1";
        Common::Log->append(tr("Database Migrated from version 1.2.0 to 1.2.1"), eLog::Success);
    }
}

void MainWindow::openPayPal()
{
    QDesktopServices::openUrl(QUrl("https://paypal.me/AlexM71100", QUrl::TolerantMode));
}

void MainWindow::openMemories()
{
    MemoriesDialog* window = new MemoriesDialog(m_savepath, this);

    if(window->result() == -1) {
        QMessageBox messageBox;
        messageBox.setWindowTitle("Memories");
        messageBox.setWindowIcon(Common::GetIconAccordingToColorScheme(qApp->styleHints()->colorScheme(), "flashback.png"));
        messageBox.setText(tr("No movie to display in Memories"));
        messageBox.setStandardButtons(QMessageBox::Ok);
        messageBox.setDefaultButton(QMessageBox::Ok);
        messageBox.setIcon(QMessageBox::Information);
        messageBox.exec();
    }
    delete window;
}
