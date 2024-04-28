#include "LogDialog.hpp"
#include "ui_LogDialog.h"

int LogDialog::instances = 0;

LogDialog::LogDialog(QWidget *parent) : QDialog(parent) {
    instances++;
    m_ui = new Ui::LogDialog;
    m_ui->setupUi(this);
    this->setWindowIcon(Common::GetIconAccordingToColorScheme(qApp->styleHints()->colorScheme(), "log.png"));

    fillList();
    QObject::connect(Common::Log, SIGNAL(logAppended()), this, SLOT(refresh()));
}

LogDialog::~LogDialog() {
    instances--;
    delete m_ui;
}

void LogDialog::fillList() {
    m_ui->listWidget->clear();

    QColor* color = new QColor();
    QBrush* brush = new QBrush(*color);

    for(int i = 0 ; i < Common::Log->size() ; i++) {
        m_ui->listWidget->addItem(Common::Log->getLog(i).sText);
        switch (Common::Log->getLog(i).eCriticity) {
            case eLog::Error:
                color->setRgb(251,42,42);
                brush->setColor(*color);
                m_ui->listWidget->item(i)->setForeground(*brush);
                break;
            case eLog::Warning:
                color->setRgb(251,183,42);
                brush->setColor(*color);
                m_ui->listWidget->item(i)->setForeground(*brush);
                break;
            case eLog::Success:
                color->setRgb(82,206,0);
                brush->setColor(*color);
                m_ui->listWidget->item(i)->setForeground(*brush);
                break;
            case eLog::Notice:
                if(Common::isThemeBright((enum eTheme)Common::Settings->value("theme").toInt()))
                {
                    color->setRgb(0,0,0);
                }
                else {
                    color->setRgb(255,255,255);
                }
                brush->setColor(*color);
                m_ui->listWidget->item(i)->setForeground(*brush);
                break;
        }
    }

    delete color;
    delete brush;
}

int LogDialog::instancesCount() {
    return instances;
}

void LogDialog::refresh() {
    this->fillList();
}
