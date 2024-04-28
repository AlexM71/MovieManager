#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QTranslator>
#include <QFile>
#include <QDir>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
#include <QFileDialog>
#include <QPixmap>
#include <QRandomGenerator>
#include <QStandardPaths>
#include <QImage>
#include <QSignalMapper>
#include <QSpinBox>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCheckBox>
#include <QStyleHints>

#include <cmath>
#include <ctime>
#include <iostream>

#include "Common.hpp"
#include "Enums.hpp"
#include "Tag.hpp"
#include "BuildFunctions.hpp"
#include "TagsScrollArea.hpp"

#include "Dialogs/AddViewDialog.hpp"
#include "Dialogs/EditViewsDialog.hpp"
#include "Dialogs/FiltersDialog.hpp"
#include "Dialogs/LogDialog.hpp"
#include "Dialogs/AboutDialog.hpp"
#include "Dialogs/OptionsDialog.hpp"
#include "Dialogs/EditMovieDialog.hpp"
#include "Dialogs/ChartsDialog.hpp"
#include "Dialogs/ChangelogDialog.hpp"
#include "Dialogs/CalendarDialog.hpp"
#include "Dialogs/MemoriesDialog.hpp"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

    private:

        Ui::MainWindow* m_ui;
        QApplication* m_app;
        QSqlDatabase m_db;
        QTranslator m_translator;
        QString m_savepath = "";
        QLocale* m_locale;
        QString m_customColumnsRequestFilter;

        TagsScrollArea* m_selectedTagsScrollArea;
        TagsScrollArea* m_movieTagsScrollArea;


    public:

        explicit MainWindow(QApplication* app);
        bool InitDatabase();
        QString getDatabaseVersion();
        void CreateTables();
        bool BackupDatabase();
        void fillGlobalStats();
        void filterTable();
        void removeUnusedTags();
        void refreshLanguage();
        void setMatrixMode(bool state);
        int getIndexOfMovie(int ID);
        int getIDOfMovie(int nIndex);
        void closeEvent(QCloseEvent *event);
        void deleteMovie(int nMovieID);
        void CheckDatabaseVersion();
        ~MainWindow();

    public slots:
        void fillTable();
        void addView(int nMovieID = -1);
        void editViews(int nMovieID = -1);
        void openFilters();
        void menuBarConnectors();
        void openLog();
        void openAbout();
        void openSettings();
        void openCharts();
        void openCalendar();
        void resetFilters();
        void importDB();
        void exportDB();
        void customMenuRequested(QPoint pos);
        void deleteMovieConfirmation(int nMovieID = -1);
        void editMovie(int nMovieID = -1);
        void fillMovieInfos(int nMovieID = -1);
        void on_EasterEggAct_triggered();
        void on_whatsnewAct_triggered();
        void on_MoviesListWidget_cellClicked(int row, int column);
        void on_MoviesListWidget_cellDoubleClicked(int row, int column);
        void on_ManageMovieViewsButton_clicked();
        void CheckForUpdates(bool bManualTrigger = true);
        void refreshTheme();
        void openPayPal();
        void openMemories();

        void clickedTag(Tag* tag);
        void clickedFilterTag(Tag* tag);
        void mouseEnteredTag(Tag* tag);
        void mouseLeftTag(Tag* tag);

};

#endif // MAINWINDOW_HPP
