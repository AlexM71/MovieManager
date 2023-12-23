#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QApplication* app) {

#ifdef DEV
    m_savepath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "\\MovieManager_Dev";
    this->setWindowTitle("MovieManager (DEV)");
#else
    m_savepath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "\\MovieManager";
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

    // Filters initialisation
    m_filters.sName = "";
    m_filters.nMinYear = 1870;
    m_filters.nMaxYear = 2023;
    m_filters.nMinRating = 0;
    m_filters.nMaxRating = 10;

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

    databaseConnection();

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
}

MainWindow::~MainWindow() {
    delete m_ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    Common::Settings->setValue("LastMovieOpened", m_ui->MoviesListWidget->item(m_ui->MoviesListWidget->currentRow(),2)->text().toInt());
}

void MainWindow::databaseConnection() {
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(m_savepath+"/movieDatabase.db");

    if(!m_db.open()) {
        Common::Log->append(tr("Can't open database"), eLog::Error);
    }
    else {
        Common::Log->append(tr("Database opened successfully"), eLog::Success);
    }

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
                                         "TextMaxLength INTEGER);";

    QSqlQuery ColumnBDQuery;

    if(!ColumnBDQuery.exec(ColumnDatabaseCreationString)) {
        Common::LogDatabaseError(&ColumnBDQuery);
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
    QString sMovieQueryRequest = "SELECT ID, Name, ReleaseYear FROM movies WHERE Name LIKE \"%" + m_filters.sName + "%\" "
                                 "AND ReleaseYear BETWEEN '"+QString::number(m_filters.nMinYear)+"' AND '"+QString::number(m_filters.nMaxYear)+"' "
                                 "AND Rating BETWEEN '"+QString::number(m_filters.nMinRating)+"' AND '"+QString::number(m_filters.nMaxRating)+"'"
                                 +m_customColumnsRequestFilter;

    Common::Log->append(tr("Fetching from database"), eLog::Notice);
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

    Common::Log->append(tr("Movies fetched from database: %1").arg(QString::number(numberOfParsedMovies)), eLog::Notice);

    /* QUICK FILTER PART*/

    QSqlQuery tagQuery;
    if(!tagQuery.exec("SELECT * FROM tags;"))
        Common::LogDatabaseError(&tagQuery);

    QString filter = m_ui->QuickSearchLineEdit->text();
    QChar cellChar, filterChar;
    int cellsNotCorrespondingToFilter;
    for(int row = 0 ; row < m_ui->MoviesListWidget->rowCount() ; row++) {
        cellsNotCorrespondingToFilter = 0;
        for(int column = 0 ; column < m_ui->MoviesListWidget->columnCount()-1 ; column++) {
            QString cellText = m_ui->MoviesListWidget->item(row, column)->text();
            if(filter.length() <= cellText.length()) {
                for(int filterIndex = 0 ; filterIndex < filter.length() ; filterIndex++) {
                        cellChar = cellText.at(filterIndex);
                        filterChar = filter.at(filterIndex);
                        if(!Common::Settings->value("quickSearchCaseSensitive").toBool()) {
                            cellChar = cellChar.toLower();
                            filterChar = filterChar.toLower();
                        }
                        if(cellChar != filterChar) {
                            cellsNotCorrespondingToFilter++;
                            break;
                        }
                    }
                }
            else {
                cellsNotCorrespondingToFilter++;
            }
        }
        //If no cell in the line corresponds to the search
        if(cellsNotCorrespondingToFilter == m_ui->MoviesListWidget->columnCount()-1) {
            m_ui->MoviesListWidget->hideRow(row);
        }
        //If cell correspond to the quick search, check if tags filter correspond
        else {
            int ID = m_ui->MoviesListWidget->item(row, 2)->text().toInt();
            bool hasMovieAllFilterTags = true;
            for(int filterTag = 0 ; filterTag < m_selectedTagsScrollArea->widget()->layout()->count()-1 ; filterTag++) {
                Tag* tag = (Tag*)m_selectedTagsScrollArea->widget()->layout()->itemAt(filterTag)->widget();
                bool hasMovieFilterTag = false;
                tagQuery.first();
                tagQuery.previous();
                while(tagQuery.next()) {
                    if(tagQuery.value(0).toInt() == ID && QString::compare(tagQuery.value(1).toString(), tag->text()) == 0) {
                        hasMovieFilterTag = true;
                        break;
                    }
                }
                if(hasMovieFilterTag == false) {
                    hasMovieAllFilterTags = false;
                    break;
                }
            }
            if(hasMovieAllFilterTags == false) {
                m_ui->MoviesListWidget->hideRow(row);
            }
        }
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

    m_ui->MovieTitleLabel->setText(m_ui->MoviesListWidget->item(m_ui->MoviesListWidget->currentRow(),0)->text());

    //Fetch the number of views of the current movie
    QSqlQuery posterQuery;
    if(!posterQuery.exec("SELECT Poster FROM movies WHERE ID='"+ID+"'"))
        Common::LogDatabaseError(&posterQuery);
    posterQuery.first();

    Common::DisplayPoster(m_ui->PosterLabel, 400, 1, m_savepath+"\\Posters\\"+posterQuery.value(0).toString());

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
            QLabel* label = new QLabel(QString(tr("%1: %2")).arg(sCustomColumnsNameList.at(nColumn), customColumnsInformationsQuery.value(nColumn).toString()));
            m_ui->CustomInfosLayout->addWidget(label);
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
    if(window->exec() == 1) {

        QString movieName;
        QString movieYear;

        //Add the new movie to the movies table
        if(window->getComboboxSelectedItem() == "") {

            movieName = window->getName();
            movieYear = QString::number(window->getReleaseYear());

            bool movieAlreadyExist = false;
            QSqlQuery existingMoviesQuery;
            if(!existingMoviesQuery.exec("SELECT Name, ReleaseYear FROM movies")) {
                Common::LogDatabaseError(&existingMoviesQuery);
                return;
            }
            while(existingMoviesQuery.next()) {
                if(QString::compare(movieName, existingMoviesQuery.value(0).toString()) == 0 && QString::compare(movieYear, existingMoviesQuery.value(1).toString()) == 0) {
                    movieAlreadyExist = true;
                    QMessageBox::information(this, tr("Movie already exists"), tr("There is already a movie with this name and release year, the view will be added to this one"));
                    break;
                }
            }
            if(movieAlreadyExist == false) {
                QString posterPath = "";
                if(window->getPosterPath() != "") {
                    QImage poster(window->getPosterPath());
                    if(poster.height() > 1200 || poster.width() > 1200) {
                        Common::Log->append(tr("Image to big (%1x%2). Latencies can be felt.").arg(QString::number(poster.width()), QString::number(poster.height())), eLog::Warning);
                    }

                    //Processing poster moving and renaming
                    QString ext = window->getPosterPath().remove(0, window->getPosterPath().lastIndexOf(".")+1);
                    QString GUID = QString::number(QRandomGenerator::global()->generate());
                    if(QFile::copy(window->getPosterPath(), m_savepath+"\\Posters/"+GUID+"."+ext) == false) {
                        Common::Log->append(tr("Error while copying poster,\nOriginal path: %1\nDestination path: %2\\Posters/%3.%4").arg(window->getPosterPath(), m_savepath, GUID, ext), eLog::Error);
                    }
                    posterPath = GUID+"."+ext;
                }

                QList<QWidget*>* customColumnsInputList = window->getCustomColumnsInputList();
                QList<QString>* customColumnsNameList = window->getCustomColumnsNameList();
                QString sRequest = "INSERT INTO movies (Name, ReleaseYear, Rating, Poster";
                for(int nColumn = 0; nColumn < customColumnsNameList->size(); nColumn++) {
                    sRequest.append(", \"" + customColumnsNameList->at(nColumn) + "\"");
                }
                sRequest.append(") VALUES (?,?,?,?");
                for(int nColumn = 0; nColumn < customColumnsNameList->size(); nColumn++) {
                    sRequest.append(",?");
                }
                sRequest.append(");");
                
		QSqlQuery insertIntoMoviesQuery;
                insertIntoMoviesQuery.prepare(sRequest);

                insertIntoMoviesQuery.bindValue(0, window->getName());
                insertIntoMoviesQuery.bindValue(1, window->getReleaseYear());
                insertIntoMoviesQuery.bindValue(2, window->getRating());
                insertIntoMoviesQuery.bindValue(3, posterPath);
                for(int nColumn = 0; nColumn < customColumnsInputList->size(); nColumn++) {
                    if(qobject_cast<QLineEdit*>(customColumnsInputList->at(nColumn)) != nullptr)
                    {
                        QLineEdit* input = qobject_cast<QLineEdit*>(customColumnsInputList->at(nColumn));

                        insertIntoMoviesQuery.bindValue(4 + nColumn, input->text());
                    }
                    else if(qobject_cast<QSpinBox*>(customColumnsInputList->at(nColumn)) != nullptr)
                    {
                        QSpinBox* input = qobject_cast<QSpinBox*>(customColumnsInputList->at(nColumn));

                        insertIntoMoviesQuery.bindValue(4 + nColumn, input->value());
                    }
                    else if(qobject_cast<QDoubleSpinBox*>(customColumnsInputList->at(nColumn)) != nullptr)
                    {
                        QDoubleSpinBox* input = qobject_cast<QDoubleSpinBox*>(customColumnsInputList->at(nColumn));

                        insertIntoMoviesQuery.bindValue(4 + nColumn, input->value());
                    }
                }

                if(!insertIntoMoviesQuery.exec()){
                    Common::LogDatabaseError(&insertIntoMoviesQuery);
                    return;
                }
            }
        }
        else {
            movieName = window->getComboboxSelectedItem().remove(window->getComboboxSelectedItem().length()-7, window->getComboboxSelectedItem().length());
            movieYear = window->getComboboxSelectedItem().remove(0, window->getComboboxSelectedItem().length()-4);
        }

        //Add the view to the views table
        QSqlQuery insertIntoViewsQuery;
        insertIntoViewsQuery.prepare("INSERT INTO views (ID_Movie, ViewDate, ViewType) VALUES (?,?,?);");


        QSqlQuery viewedMovieIDQuery;
        if(!viewedMovieIDQuery.exec("SELECT ID FROM movies WHERE Name=\""+movieName+"\" AND ReleaseYear=\""+movieYear+"\"")) {
            Common::LogDatabaseError(&viewedMovieIDQuery);
            return;
        }
        viewedMovieIDQuery.first();

        int nNewMovie = viewedMovieIDQuery.value(0).toInt();

        QString ViewDate;
        int ViewType;

        if(window->isDateUnknown()) {
            ViewDate = "?";
        }
        else {
            ViewDate = window->getViewDate().toString("yyyy-MM-dd");
        }

        if(window->isTypeUnknown()) {
            ViewType = eViewType::Unknown;
        }
        else {
            ViewType = window->getViewType();
        }

        insertIntoViewsQuery.bindValue(0, nNewMovie);
        insertIntoViewsQuery.bindValue(1, ViewDate);
        insertIntoViewsQuery.bindValue(2, ViewType);

        if(!insertIntoViewsQuery.exec()){
            Common::LogDatabaseError(&insertIntoViewsQuery);
            return;
        }

        for(int i=0 ; i<window->getTags()->size() ; i++) {
            QString hexcolor = "";
            for(int j = 0 ; j < 6 ; j++) {
                hexcolor.append(QString::number(rand() % 9));
            }

            QSqlQuery insertIntoTagsInfoQuery;

            insertIntoTagsInfoQuery.prepare("INSERT INTO tagsInfo (Tag, Color) VALUES (?,?);");
            insertIntoTagsInfoQuery.bindValue(0, window->getTags()->at(i));
            insertIntoTagsInfoQuery.bindValue(1, hexcolor);

            if(!insertIntoTagsInfoQuery.exec()){
                Common::LogDatabaseError(&insertIntoTagsInfoQuery);
                return;
            }

            QSqlQuery insertIntoTagsQuery;

            insertIntoTagsQuery.prepare("INSERT INTO tags (ID_Movie, Tag) VALUES (?,?);");
            insertIntoTagsQuery.bindValue(0, nNewMovie);
            insertIntoTagsQuery.bindValue(1, window->getTags()->at(i));

            if(!insertIntoTagsQuery.exec()){
                Common::LogDatabaseError(&insertIntoTagsQuery);
                return;
            }

        }
        fillGlobalStats();
        fillTable();
        if(nMovieID == -1) {
            m_ui->MoviesListWidget->setCurrentCell(getIndexOfMovie(nNewMovie), 0);
            fillMovieInfos(nNewMovie);
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
                    QFile::remove(m_savepath+"\\Posters\\"+MoviePosterToDeleteQuery.value(0).toString());

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


        if(window->newPoster()) {
            //Delete old poster
            QSqlQuery posterQuery;
            if(!posterQuery.exec("SELECT Poster FROM movies WHERE ID=\""+ID+"\";"))
                Common::LogDatabaseError(&posterQuery);
            posterQuery.first();
            QFile::remove(m_savepath+"\\Posters\\"+posterQuery.value(0).toString());

            ext = window->getPosterPath().remove(0, window->getPosterPath().lastIndexOf(".")+1);

            GUID = QString::number(QRandomGenerator::global()->generate());
            if(QFile::copy(window->getPosterPath(), m_savepath+"\\Posters/"+GUID+"."+ext) == false) {
                Common::Log->append(tr("Error while copying poster,\nOriginal path: %1\nDestination path: %2\\Posters/%3.%4").arg(window->getPosterPath(), m_savepath, GUID, ext), eLog::Error);
            }

            // Add poster ID modification to the update request
            sUpdateMovieRequest += ", Poster=\""+GUID+"."+ext+"\"";
        }
        QSqlQuery editMovieQuery;

        QList<QWidget*>* customColumnsInputList = window->getCustomColumnsInputList();
        QList<QString>* customColumnsNameList = window->getCustomColumnsNameList();

        QString sUpdateMovieRequest = "UPDATE movies SET Name=\""+window->getMovieName()+"\", ReleaseYear=\""+window->getReleaseYear()+
                              "\", Rating=\""+QString::number(window->getRating())+"\" ";
        for(int nColumn = 0; nColumn < customColumnsNameList->size(); nColumn++) {
            if(qobject_cast<QLineEdit*>(customColumnsInputList->at(nColumn)) != nullptr)
            {
                QLineEdit* input = qobject_cast<QLineEdit*>(customColumnsInputList->at(nColumn));
                sUpdateMovieRequest.append(", \"" + customColumnsNameList->at(nColumn) + "\"=\"" + input->text() + "\"");

            }
            else if(qobject_cast<QSpinBox*>(customColumnsInputList->at(nColumn)) != nullptr)
            {
                QSpinBox* input = qobject_cast<QSpinBox*>(customColumnsInputList->at(nColumn));
                sUpdateMovieRequest.append(", \"" + customColumnsNameList->at(nColumn) + "\"=\"" + QString::number(input->value()) + "\"");

            }
            else if(qobject_cast<QDoubleSpinBox*>(customColumnsInputList->at(nColumn)) != nullptr)
            {
                QDoubleSpinBox* input = qobject_cast<QDoubleSpinBox*>(customColumnsInputList->at(nColumn));
                sUpdateMovieRequest.append(", \"" + customColumnsNameList->at(nColumn) + "\"=\"" + QString::number(input->value()) + "\"");

            }
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

void MainWindow::deleteMovie(int nMovieID) {
    QMessageBox::StandardButton reply;
    int savedRow = 0;
    reply = QMessageBox::question(this, tr("Remove the movie"), tr("Are you sure do you want to remove the movie? Its views will be removed."));
    if(reply == QMessageBox::Yes) {

        QSqlQuery deleteMovieQuery;
        QSqlQuery deleteAssociatedViewsQuery;
        QSqlQuery deleteAssociatedTagsQuery;
        QSqlQuery posterQuery;

        QString ID = QString::number(nMovieID);
        savedRow = m_ui->MoviesListWidget->currentRow();

        if(!posterQuery.exec("SELECT Poster FROM movies WHERE ID=\""+ID+"\";"))
            Common::LogDatabaseError(&posterQuery);

        posterQuery.first();
        QFile::remove(m_savepath+"\\Posters\\"+posterQuery.value(0).toString());


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
}

void MainWindow::openFilters() {
    FiltersDialog* window = new FiltersDialog(&m_filters);
    window->show();
    if(window->exec() == 1) {
        m_ui->ResetFiltersButton->setEnabled(true);

        QList<QString> sColumnsList;
        for(auto value : window->getCustomColumnsMap()->values()) {
            if(sColumnsList.contains(value) == false)
                sColumnsList.append(value);
        }

        m_customColumnsRequestFilter = "";
        for(QString sColumn : sColumnsList) {
            QSqlQuery sColumnTypeQuery;
            if(sColumnTypeQuery.exec("SELECT Type FROM columns WHERE Name=\"" + sColumn + "\"") == false) {
                Common::LogDatabaseError(&sColumnTypeQuery);
                continue;
            }
            sColumnTypeQuery.first();

            QList<QWidget*> widgetList;
            for(auto key : window->getCustomColumnsMap()->keys(sColumn)) {
                widgetList.append(key);
            }

            if(sColumnTypeQuery.value(0).toInt() == 0) { // Int
                QSpinBox* lowValue = qobject_cast<QSpinBox*>(widgetList.at(0));
                QSpinBox* highValue = qobject_cast<QSpinBox*>(widgetList.at(1));
                int nLowValue = std::min(lowValue->value(), highValue->value());
                int nHighValue = std::max(lowValue->value(), highValue->value());
                m_customColumnsRequestFilter.append(" AND \"" + sColumn + "\" BETWEEN \"" + QString::number(nLowValue) + "\" AND \"" + QString::number(nHighValue) + "\"");
            }
            else if(sColumnTypeQuery.value(0).toInt() == 1) { // Double
                QDoubleSpinBox* lowValue = qobject_cast<QDoubleSpinBox*>(widgetList.at(0));
                QDoubleSpinBox* highValue = qobject_cast<QDoubleSpinBox*>(widgetList.at(1));
                double nLowValue = std::min(lowValue->value(), highValue->value());
                double nHighValue = std::max(lowValue->value(), highValue->value());
                m_customColumnsRequestFilter.append(" AND \"" + sColumn + "\" BETWEEN \"" + QString::number(nLowValue) + "\" AND \"" + QString::number(nHighValue) + "\"");
            }
            else if(sColumnTypeQuery.value(0).toInt() == 2) { // Text
                QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widgetList.at(0));
                m_customColumnsRequestFilter.append(" AND \"" + sColumn + "\" LIKE \"%" + lineEdit->text() + "%\"");
            }
        }

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
    m_filters.sName = "";
    m_filters.nMinYear = 1870;
    m_filters.nMaxYear = 2023;
    m_filters.nMinRating = 0;
    m_filters.nMaxRating = 10;
    m_customColumnsRequestFilter = "";
    fillTable();
}

void MainWindow::customMenuRequested(QPoint pos) {
    if(m_ui->MoviesListWidget->currentRow() == -1)
        return;

    int nMovieID = m_ui->MoviesListWidget->item(m_ui->MoviesListWidget->currentRow(),2)->text().toInt();
    fillMovieInfos(nMovieID);

    QMenu *menu = new QMenu(this);

    QAction* addViewAction = new QAction(tr("Add a view"), this);
    Common::setIconAccordingToTheme(addViewAction, "plus.png");

    QAction* editAction = new QAction(tr("Edit"), this);
    Common::setIconAccordingToTheme(editAction, "edit.png");

    QAction* deleteAction = new QAction(tr("Remove"), this);
    Common::setIconAccordingToTheme(deleteAction, "delete.png");


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
    QObject::connect(deleteMovieSignalMapper, SIGNAL(mappedInt(int)), this, SLOT(deleteMovie(int)));
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

    switch(Common::Settings->value("language").toInt()) {
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

    switch(Common::Settings->value("theme").toInt()) {
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
    Common::setIconAccordingToTheme(m_ui->ExportAct, "export.png");
    Common::setIconAccordingToTheme(m_ui->ImportAct, "import.png");
    Common::setIconAccordingToTheme(m_ui->QuitAct, "exit.png");
    Common::setIconAccordingToTheme(m_ui->OptionsAct, "settings.png");
    Common::setIconAccordingToTheme(m_ui->LogAct, "log.png");
    Common::setIconAccordingToTheme(m_ui->ChartAct, "chart.png");
    Common::setIconAccordingToTheme(m_ui->CalendarAct, "calendar.png");
    Common::setIconAccordingToTheme(m_ui->whatsnewAct, "github.png");
    Common::setIconAccordingToTheme(m_ui->AboutAct, "info.png");
    Common::setIconAccordingToTheme(m_ui->CheckForUpdateAct, "download.png");
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
    for(int i = 0 ; i < m_selectedTagsScrollArea->widget()->layout()->count() - 2 ; i++) {
        Tag* t = (Tag*)m_selectedTagsScrollArea->widget()->layout()->itemAt(i)->widget();
        if(QString::compare(t->text(), tag->text()) == 0) {
            isTagInLayout = true;
            break;
        }
    }

    if(!isTagInLayout) {
        Tag* copiedTag = new Tag(tag);

        QObject::connect(copiedTag, SIGNAL(clicked(Tag*)), this, SLOT(clickedFilterTag(Tag*)));

        QLayoutItem* spacer = m_selectedTagsScrollArea->widget()->layout()->takeAt(m_selectedTagsScrollArea->widget()->layout()->count()-1);
        m_selectedTagsScrollArea->widget()->layout()->addWidget(copiedTag);
        m_selectedTagsScrollArea->widget()->layout()->addItem(spacer);
        fillTable();
    }

}

void MainWindow::clickedFilterTag(Tag* tag) {
    delete tag;
    // Widget height forced to 0 to avoid a vertical space if no tags are selected
    if(m_selectedTagsScrollArea->widget()->layout()->count() == 1)
        m_selectedTagsScrollArea->setMaximumHeight(0);
    fillTable();
}

void MainWindow::on_MoviesListWidget_cellClicked(int row, int column) {
    fillMovieInfos(m_ui->MoviesListWidget->item(row,2)->text().toInt());
}

void MainWindow::on_MoviesListWidget_cellDoubleClicked(int row, int column) {
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


