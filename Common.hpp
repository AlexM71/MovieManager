#ifndef COMMON_HPP
#define COMMON_HPP

#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QFileInfo>
#include <QLayoutItem>
#include <QLayout>
#include <QSettings>
#include <QSqlQuery>
#include <QSqlError>
#include <QPushButton>
#include <QApplication>
#include <QStyleHints>

#include "Enums.hpp"
#include "Log.hpp"
#include "BuildFunctions.hpp"

class Common
{
public:

    Common();

    static Log* Log;
    static QSettings* Settings;

    static QString SelectPoster(QWidget* parent = nullptr);
    static void DisplayPoster(QLabel* poster, int nPosterHeight, float fSafeRatio, QString sPath);

    static void ratingToStar(int rating, QLabel* ratingLabel);

    static void clearLayout(QLayout* layout);

    static QString viewTypeToQString(enum eViewType type);

    static enum eViewType QStringToViewType(QString type);

    static QString ColumnTypeToQString(enum eColumnType type);

    static QIcon GetIconAccordingToColorScheme(Qt::ColorScheme scheme, QString sIconFile);
    static QIcon GetIconAccordingToTheme(enum eTheme theme, QString sIconFile);
    static bool isThemeBright(enum eTheme);

    static QString getVersion();
    static QString getPreviousVersion();

    static void LogDatabaseError(QSqlQuery *query);
};

#endif // COMMON_HPP
