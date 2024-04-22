#ifndef LOGDIALOG_HPP
#define LOGDIALOG_HPP

#include <QDialog>

#include "Common.hpp"

namespace Ui {
class LogDialog;
}

class LogDialog : public QDialog
{
    Q_OBJECT

    private:
        Ui::LogDialog* m_ui;
        static int instances;

    public:

        explicit LogDialog(QWidget *parent = nullptr);
        ~LogDialog();

        void fillList();
        static int instancesCount();

    public slots:
        void refresh();


};

#endif // LOGDIALOG_HPP
