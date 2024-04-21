#ifndef CALENDARDIALOG_H
#define CALENDARDIALOG_H

#include <QDialog>
#include <QDate>
#include <QGridLayout>
#include <QLabel>
#include <QStringList>
#include <QSqlQuery>

#include "Common.h"

namespace Ui {
    class CalendarDialog;
}

class CalendarDialog : public QDialog
{
    Q_OBJECT

    private:
        Ui::CalendarDialog* m_ui;
        QDate m_selectedDate;
        int m_daysPerMonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        QStringList m_monthLabel = {
            tr("January"),
            tr("February"),
            tr("March"),
            tr("April"),
            tr("May"),
            tr("June"),
            tr("July"),
            tr("August"),
            tr("September"),
            tr("October"),
            tr("November"),
            tr("December")
        };

        // List that contains every displayed labels
        QList<QLabel*>* m_labels;

        static int instances;

        void deleteLabels();

    public:
        explicit CalendarDialog(QWidget *parent = nullptr);
        ~CalendarDialog();

        void setDisplay();
        void setData();
        int getOffset();

        static int instancesCount();
        void resizeEvent(QResizeEvent* event);

    public slots:
        void previousButtonClicked();
        void nextButtonClicked();

};

#endif // CALENDARDIALOG_H
