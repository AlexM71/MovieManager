#ifndef CHANGELOGDIALOG_HPP
#define CHANGELOGDIALOG_HPP

#include <QDialog>
#include <QFile>
#include <QLabel>

namespace Ui {
    class ChangelogDialog;
}

class ChangelogDialog : public QDialog
{
    Q_OBJECT

    private:
        Ui::ChangelogDialog* m_ui;
        static int m_instances;

    public:
        explicit ChangelogDialog(QWidget *parent = nullptr);
        ~ChangelogDialog();

        static int instancesCount();

    public slots:
        void loadHTML();

};

#endif // CHANGELOGDIALOG_HPP
