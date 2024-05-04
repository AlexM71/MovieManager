#include "ChangelogDialog.hpp"
#include "ui_ChangelogDialog.h"

int ChangelogDialog::m_instances = 0;

ChangelogDialog::ChangelogDialog(QString sSavepath, QWidget *parent) : QDialog(parent) {
    m_instances++;
    m_ui = new Ui::ChangelogDialog;
    m_ui->setupUi(this);
    this->setWindowIcon(Common::GetIconAccordingToColorScheme(qApp->styleHints()->colorScheme(), "github.png"));
    m_pSavepath = sSavepath + QDir::separator() + "changelog_" + Common::getVersion() + ".md";

    m_ui->TitleLabel->setText(tr("What's new in MovieManager %1").arg(Common::getVersion()));


    QFile changelog(m_pSavepath);

    if(changelog.exists())
    {
        qDebug() << "Récupération des commits depuis une sauvegarde locale";
        if(changelog.open(QIODevice::ReadOnly))
        {
            QString sResponse;
            QTextStream in(&changelog);
            while(!in.atEnd()) {
                sResponse += in.readLine();
            }

            m_responseByteArray = sResponse.toLatin1();
            LoadLastCommits();
        }
    }
    else
    {
        qDebug() << "Récupération des commits depuis l'API";
        QNetworkAccessManager* manager = new QNetworkAccessManager(this);
        QNetworkReply* reply = manager->get(QNetworkRequest(QUrl("https://api.github.com/repos/AmbreM71/MovieManager/compare/" + Common::getPreviousVersion() + "..." + Common::getVersion())));
        QObject::connect(reply, &QNetworkReply::finished, [=]() {
            QFile changelog(m_pSavepath);
            m_responseByteArray = reply->readAll();
            LoadLastCommits();

            // Write API answer to file
            if(changelog.open(QIODevice::WriteOnly))
                changelog.write(m_responseByteArray);

            reply->deleteLater();
            delete manager;
        });
    }

}

ChangelogDialog::~ChangelogDialog() {
    m_instances--;
    delete m_ui;
}

int ChangelogDialog::instancesCount() {
    return m_instances;
}

void ChangelogDialog::LoadLastCommits()
{
    QStringList sCommitsMessages;
    QJsonDocument response = QJsonDocument::fromJson(m_responseByteArray);
    QJsonArray commits = response["commits"].toArray();
    for(int nCommit = 0; nCommit < commits.count(); nCommit++)
    {
        QJsonObject commit = commits.at(nCommit).toObject();
        QJsonObject commitContent = commit["commit"].toObject();
        QString sComment = commitContent.value("message").toString();
        sComment = sComment.left(sComment.indexOf(QChar('\n')));
        sCommitsMessages.append(sComment + "\n");
    }

    m_ui->commitsLabel->setText(sCommitsMessages.join('\n'));
}

