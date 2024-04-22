#include "AboutDialog.hpp"
#include "ui_AboutDialog.h"

short AboutDialog::instances = 0;

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent) {
    instances++;
    m_ui = new Ui::AboutDialog;
    m_ui->setupUi(this);
    this->setWindowIcon(QIcon(":/assets/Assets/Icons/Dark/info.png"));

    m_ui->QtVersionLabel->setText(tr("Powered by Qt %1").arg(qVersion()));
#ifdef DEV
    m_ui->VersionLabel->setText("Version develop");
#else
    m_ui->VersionLabel->setText(tr("Version %1").arg(Common::getVersion()));
#endif

    QObject::connect(m_ui->GithubButton, SIGNAL(clicked()), this, SLOT(redirectGithub()));
}

AboutDialog::~AboutDialog() {
    instances--;
    delete m_ui;
}

short AboutDialog::instancesCount() {
    return instances;
}

void AboutDialog::redirectGithub() {
    QDesktopServices::openUrl(QUrl("https://github.com/AmbreM71/MovieManager", QUrl::TolerantMode));
}
