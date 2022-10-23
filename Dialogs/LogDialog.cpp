#include "LogDialog.h"
#include "ui_LogDialog.h"

int LogDialog::instances = 0;

LogDialog::LogDialog(Log* log,  int* theme, QWidget *parent) : QDialog(parent) {
    instances++;
    m_ui = new Ui::LogDialog;
    m_ui->setupUi(this);
    m_log = log;
    m_theme = theme;
    fillList();
    QObject::connect(m_log, SIGNAL(logAppended()), this, SLOT(refresh()));
}

LogDialog::~LogDialog() {
    instances--;
    delete m_ui;
}

void LogDialog::fillList() {
    m_ui->listWidget->clear();

    QColor* color = new QColor();
    QBrush* brush = new QBrush(*color);

    for(int i = 0 ; i < m_log->size() ; i++) {
        m_ui->listWidget->addItem(m_log->getLog(i).string);
        switch (m_log->getLog(i).criticity) {
            case Error:
                color->setRgb(251,42,42);
                brush->setColor(*color);
                m_ui->listWidget->item(i)->setForeground(*brush);
                break;
            case Warning:
                color->setRgb(251,183,42);
                brush->setColor(*color);
                m_ui->listWidget->item(i)->setForeground(*brush);
                break;
            case Success:
                color->setRgb(82,206,0);
                brush->setColor(*color);
                m_ui->listWidget->item(i)->setForeground(*brush);
                break;
            case Notice:
                if (*m_theme == Theme::Classic) {
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
