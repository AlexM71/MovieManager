#ifndef CHANGELOGDIALOG_HPP
#define CHANGELOGDIALOG_HPP

#include <QDialog>
#include <QFile>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "Common.hpp"

namespace Ui {
    class ChangelogDialog;
}

class ChangelogDialog : public QDialog
{
    Q_OBJECT

    private:
        Ui::ChangelogDialog* m_ui;
        static int m_instances;
        QByteArray m_responseByteArray;
        QString m_pSavepath;

    public:
        explicit ChangelogDialog(QString sSavepath, QWidget *parent = nullptr);
        ~ChangelogDialog();

        static int instancesCount();

    public slots:
        void LoadLastCommits();

};

#endif // CHANGELOGDIALOG_HPP
