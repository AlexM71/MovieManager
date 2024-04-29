#include "OptionsDialog.hpp"
#include "ui_OptionsDialog.h"

OptionsDialog::OptionsDialog(QWidget *parent) : QDialog(parent) {
    m_ui = new Ui::OptionsDialog;
    m_ui->setupUi(this);
    this->setWindowIcon(Common::GetIconAccordingToColorScheme(qApp->styleHints()->colorScheme(), "settings.png"));

    pPreviousWidget = m_ui->RatingButton;

    // Set current settings values
    m_ui->ThemeCombobox->setCurrentIndex(Common::Settings->value("theme").toInt());
    m_ui->LanguageCombobox->setCurrentIndex(Common::Settings->value("language").toInt());
    m_ui->MatrixModeCheckbox->setChecked(Common::Settings->value("matrixMode").toBool());
    m_ui->QuickSearchCaseCheckbox->setChecked(Common::Settings->value("quickSearchCaseSensitive").toBool());
    m_ui->MoreLogsCheckbox->setChecked(Common::Settings->value("moreLogs").toBool());
    m_ui->DateFormatCombobox->setCurrentIndex(m_ui->DateFormatCombobox->findText(Common::Settings->value("dateFormat").toString()));

    m_ui->scrollAreaLayout->addStretch(100);

    // Add custom columns to the list
    QSqlQuery columnsQuery;
    if(!columnsQuery.exec("SELECT Name, Type FROM columns ORDER BY Name"))
        Common::LogDatabaseError(&columnsQuery);

    m_editColumnSignalMapper = new QSignalMapper();
    m_deleteColumnSignalMapper = new QSignalMapper();

    int nRow = 0;
    while(columnsQuery.next()) {
        InsertColumnQt(columnsQuery.value(0).toString(), (enum eColumnType)columnsQuery.value(1).toInt(), 4 + nRow);
        nRow++;
    }

    //QWidget::setTabOrder(pPreviousWidget, m_ui->AddColumnButton);

    QObject::connect(m_editColumnSignalMapper, SIGNAL(mappedInt(int)), this, SLOT(EditColumn(int)));
    QObject::connect(m_deleteColumnSignalMapper, SIGNAL(mappedInt(int)), this, SLOT(RemoveColumn(int)));

    QObject::connect(m_ui->AddColumnButton, SIGNAL(clicked()), this, SLOT(AddColumn()));
}

OptionsDialog::~OptionsDialog() {
    Common::Settings->setValue("language", m_ui->LanguageCombobox->currentIndex());
    Common::Settings->setValue("theme", m_ui->ThemeCombobox->currentIndex());
    Common::Settings->setValue("matrixMode", m_ui->MatrixModeCheckbox->isChecked());
    Common::Settings->setValue("quickSearchCaseSensitive", m_ui->QuickSearchCaseCheckbox->isChecked());
    Common::Settings->setValue("moreLogs", m_ui->MoreLogsCheckbox->isChecked());
    Common::Settings->setValue("dateFormat", m_ui->DateFormatCombobox->currentText());
    delete m_ui;
}


void OptionsDialog::AddColumn() {
    AddColumnDialog* window = new AddColumnDialog(this);
    window->show();
    if(window->exec() == 1) {
        InsertColumn(window->getColumn(), m_ui->DefaultColumnGridLayout->rowCount());
    }
    delete window;
}

void OptionsDialog::EditColumn(int nRow) {
    QString sName = qobject_cast<QLabel*>(m_ui->DefaultColumnGridLayout->itemAtPosition(nRow,0)->widget())->text();
    QSqlQuery columnsQuery;
    if(!columnsQuery.exec("SELECT Name, Type, Min, Max, Precision, TextMaxLength, Optional FROM columns WHERE Name = \"" + sName + "\""))
        Common::LogDatabaseError(&columnsQuery);
    columnsQuery.first();

    struct stColumn stColumnToEdit;
    stColumnToEdit.sName = columnsQuery.value(0).toString();
    stColumnToEdit.eType = (enum eColumnType)columnsQuery.value(1).toInt();
    stColumnToEdit.nMin = columnsQuery.value(2).toDouble();
    stColumnToEdit.nMax = columnsQuery.value(3).toDouble();
    stColumnToEdit.nPrecision = columnsQuery.value(4).toInt();
    stColumnToEdit.textMaxLength = columnsQuery.value(5).toInt();
    stColumnToEdit.bOptional = columnsQuery.value(6).toBool();

    AddColumnDialog* window = new AddColumnDialog(this, &stColumnToEdit);
    window->show();
    if(window->exec() == 1) {
        RemoveColumn(nRow);
        InsertColumn(window->getColumn(), m_ui->DefaultColumnGridLayout->rowCount());
    }
}

void OptionsDialog::RemoveColumn(int nRow) {
    QString sName = qobject_cast<QLabel*>(m_ui->DefaultColumnGridLayout->itemAtPosition(nRow,0)->widget())->text();
    RemoveColumnQt(nRow);
    RemoveColumnDB(sName);
}

void OptionsDialog::RemoveColumnQt(int nRow) {

    QLabel* label = qobject_cast<QLabel*>(m_ui->DefaultColumnGridLayout->itemAtPosition(nRow,0)->widget());
    QLabel* sType = qobject_cast<QLabel*>(m_ui->DefaultColumnGridLayout->itemAtPosition(nRow,1)->widget());
    QHBoxLayout* pActionLayout = (QHBoxLayout*)(m_ui->DefaultColumnGridLayout->itemAtPosition(nRow,2)->layout());

    QPushButton* editButton = qobject_cast<QPushButton*>(pActionLayout->itemAt(0)->widget());
    QPushButton* deleteButton = qobject_cast<QPushButton*>(pActionLayout->itemAt(1)->widget());

    m_ui->DefaultColumnGridLayout->removeWidget(label);
    m_ui->DefaultColumnGridLayout->removeWidget(sType);
    m_ui->DefaultColumnGridLayout->removeWidget(editButton);
    m_ui->DefaultColumnGridLayout->removeWidget(deleteButton);
    m_ui->DefaultColumnGridLayout->removeItem(pActionLayout);

    delete label;
    delete sType;
    delete editButton;
    delete deleteButton;
    delete pActionLayout;
}

void OptionsDialog::RemoveColumnDB(QString sName) {
    QSqlQuery deleteColumnsQuery;
    if(!deleteColumnsQuery.exec("DELETE FROM columns WHERE Name = \"" + sName + "\""))
        Common::LogDatabaseError(&deleteColumnsQuery);
}

void OptionsDialog::InsertColumn(struct stColumn* stColumnToInsert, int nRow) {
    InsertColumnQt(stColumnToInsert->sName, stColumnToInsert->eType, nRow);
    InsertColumnDB(stColumnToInsert);
}

void OptionsDialog::InsertColumnQt(QString sName, enum eColumnType eType, int nRow) {
    QLabel* NameLabel = new QLabel(sName);
    QLabel* TypeLabel = new QLabel(Common::ColumnTypeToQString(eType));
    QPushButton* editButton = new QPushButton(tr("Edit"));
    QPushButton* deleteButton = new QPushButton();
    deleteButton->setIcon(Common::GetIconAccordingToTheme((enum eTheme)Common::Settings->value("theme").toInt(), "delete.png"));
    deleteButton->setStyleSheet("min-width: 0px;");
    deleteButton->setMaximumWidth(30);

    QHBoxLayout* pActionsLayout = new QHBoxLayout();

    pActionsLayout->addWidget(editButton);
    pActionsLayout->addWidget(deleteButton);

    //QWidget::setTabOrder(pPreviousWidget, editButton);
    //QWidget::setTabOrder(editButton, deleteButton);
    //pPreviousWidget = deleteButton;


    m_ui->DefaultColumnGridLayout->addWidget(NameLabel, nRow, 0);
    m_ui->DefaultColumnGridLayout->addWidget(TypeLabel, nRow, 1);
    m_ui->DefaultColumnGridLayout->addLayout(pActionsLayout, nRow, 2);

    m_editColumnSignalMapper->setMapping(editButton, nRow);
    QObject::connect(editButton, SIGNAL(clicked()), m_editColumnSignalMapper, SLOT(map()));

    m_deleteColumnSignalMapper->setMapping(deleteButton, NameLabel->text());
    m_deleteColumnSignalMapper->setMapping(deleteButton, nRow);
    QObject::connect(deleteButton, SIGNAL(clicked()), m_deleteColumnSignalMapper, SLOT(map()));
}

void OptionsDialog::InsertColumnDB(struct stColumn* stColumnToInsert) {
    QSqlQuery insertColumnQuery;
    switch(stColumnToInsert->eType) {
    case eColumnType::Double:
        insertColumnQuery.prepare("INSERT INTO columns (Name, Type, Min, Max, Precision, Optional) VALUES (?,?,?,?,?,?);");
        insertColumnQuery.bindValue(0, stColumnToInsert->sName);
        insertColumnQuery.bindValue(1, (int)stColumnToInsert->eType);
        insertColumnQuery.bindValue(2, stColumnToInsert->nMin);
        insertColumnQuery.bindValue(3, stColumnToInsert->nMax);
        insertColumnQuery.bindValue(4, stColumnToInsert->nPrecision);
        insertColumnQuery.bindValue(5, (int)stColumnToInsert->bOptional);
        break;
    case eColumnType::Integer:
        insertColumnQuery.prepare("INSERT INTO columns (Name, Type, Min, Max, Optional) VALUES (?,?,?,?,?);");
        insertColumnQuery.bindValue(0, stColumnToInsert->sName);
        insertColumnQuery.bindValue(1, (int)stColumnToInsert->eType);
        insertColumnQuery.bindValue(2, stColumnToInsert->nMin);
        insertColumnQuery.bindValue(3, stColumnToInsert->nMax);
        insertColumnQuery.bindValue(4, (int)stColumnToInsert->bOptional);
        break;
    case eColumnType::Text:
        insertColumnQuery.prepare("INSERT INTO columns (Name, Type, TextMaxLength, Optional) VALUES (?,?,?,?);");
        insertColumnQuery.bindValue(0, stColumnToInsert->sName);
        insertColumnQuery.bindValue(1, (int)stColumnToInsert->eType);
        insertColumnQuery.bindValue(2, stColumnToInsert->textMaxLength);
        insertColumnQuery.bindValue(3, (int)stColumnToInsert->bOptional);
        break;
    }

    if(!insertColumnQuery.exec()){
        Common::LogDatabaseError(&insertColumnQuery);
        return;
    }
}

