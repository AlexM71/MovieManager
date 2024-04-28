#include "ChangelogDialog.hpp"
#include "ui_ChangelogDialog.h"

int ChangelogDialog::m_instances = 0;

ChangelogDialog::ChangelogDialog(QWidget *parent) : QDialog(parent) {
    m_instances++;
    m_ui = new Ui::ChangelogDialog;
    m_ui->setupUi(this);
    this->setWindowIcon(Common::GetIconAccordingToColorScheme(qApp->styleHints()->colorScheme(), "github.png"));
    loadHTML();
    //response = manager.get(QNetworkRequest(QUrl("https://github.com/AlexM71/MovieManager/commits/master")));
    //QObject::connect(response, SIGNAL(finished()), this, SLOT(loadHTML()));

}

ChangelogDialog::~ChangelogDialog() {
    m_instances--;
    delete m_ui;
}

int ChangelogDialog::instancesCount() {
    return m_instances;
}

void ChangelogDialog::loadHTML() {

    //Workaround until solution found

    QFile lastCommits("lastcommits.txt");
    if (!lastCommits.open(QIODevice::ReadOnly | QIODevice::Text))
           return;
    QTextStream in(&lastCommits);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QLabel* commit = new QLabel(line);
            m_ui->CommitsLayout->addWidget(commit);
        }
}
