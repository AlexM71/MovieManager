#ifndef MEMORIESDIALOG_H
#define MEMORIESDIALOG_H

#include <QDialog>
#include <QSqlQuery>
#include <QPainter>
#include <QWheelEvent>
#include <QPropertyAnimation>

#include "Common.hpp"

namespace Ui
{
    class Memories;
}

enum class eTransition
{
    None,
    Next,
    Previous
};

enum class eDisplay
{
    FirstViewed,
    LastViewed,
    ViewedXTimes
};

struct stMovie
{
    QString sName;
    int nID;
    QString sPosterPath;
    QList<short>* nYear; // Year(s) where this movie has been saw
    enum eDisplay display;
};

class MemoriesDialog : public QDialog
{
    Q_OBJECT

    private:
        Ui::Memories* m_ui;
        QList<stMovie>* m_movies;
        QString m_sSavePath;
        int m_displayedIndex;
        QElapsedTimer m_Timer;

        void wheelEvent(QWheelEvent *event);
        int GetMovieIndex(short nID);
        void UpdateBackgrounds(enum eTransition eTransition);
        QPixmap ResizeImage(int* nOffsetX, int* nOffsetY, int nIndex);
        void AddMovieOverlay(QPixmap* pixmap);

    public:
        explicit MemoriesDialog(QString sSavePath, QWidget *parent = nullptr);
        ~MemoriesDialog();

        void GetEphemeris();

};

#endif // MEMORIESDIALOG_H
