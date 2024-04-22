#ifndef ABOUTDIALOG_HPP
#define ABOUTDIALOG_HPP

#include <QDialog>
#include <QDesktopServices>
#include <QtVersion>

#include "Common.hpp"

namespace Ui {
    class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

    private:
        Ui::AboutDialog* m_ui;
        static short instances;

    public:
        explicit AboutDialog(QWidget *parent = nullptr);
        ~AboutDialog();

        static short instancesCount();

    public slots:
        void redirectGithub();


};

#endif // ABOUTDIALOG_HPP
