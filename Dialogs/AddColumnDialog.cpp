#include "AddColumnDialog.hpp"
#include "ui_AddColumnDialog.h"

AddColumnDialog::AddColumnDialog(QWidget *parent, struct stColumn* stColumnToEdit) : QDialog(parent) {
    m_ui = new Ui::AddColumnDialog;
    m_ui->setupUi(this);

    if(stColumnToEdit == nullptr)
        m_ui->WarningLabel->setVisible(false);
    m_ui->WarningLabel->setStyleSheet("color:#ce0000");

    m_ui->DoublePropertiesWidget->setVisible(false);
    m_ui->TextPropertiesWidget->setVisible(false);
    m_ui->IntPropertiesWidget->setVisible(false);

    // Adjust window size to fit the biggest case (double properties)
    m_ui->DoublePropertiesWidget->setVisible(true);
    this->adjustSize();
    m_ui->DoublePropertiesWidget->setVisible(false);
    m_ui->IntPropertiesWidget->setVisible(true);


    sColumnNameList << tr("Name") << tr("Release year") << tr("Rating") << tr("Poster");

    QSqlQuery columnsQuery;
    if(!columnsQuery.exec("SELECT Name FROM columns"))
        Common::LogDatabaseError(&columnsQuery);

    while(columnsQuery.next()) {
        // If a column is edited, it must not be in the list of unauthorized name
        if(stColumnToEdit != nullptr)
            if(QString::compare(columnsQuery.value(0).toString(), stColumnToEdit->sName) == 0)
                continue;
        sColumnNameList << columnsQuery.value(0).toString();
    }

    if(stColumnToEdit != nullptr) {
        m_ui->IntPropertiesWidget->setVisible(false);

        m_ui->NameLineEdit->setText(stColumnToEdit->sName);
        m_ui->TypeComboBox->setCurrentIndex((int)stColumnToEdit->eType);
        switch(stColumnToEdit->eType) {
            case eColumnType::Integer:
                m_ui->IntPropertiesWidget->setVisible(true);
                m_ui->IntMinValue->setValue(stColumnToEdit->nMin);
                m_ui->IntMaxValue->setValue(stColumnToEdit->nMax);
                break;
            case eColumnType::Double:
                m_ui->DoublePropertiesWidget->setVisible(true);
                m_ui->DoubleMinValue->setValue(stColumnToEdit->nMin);
                m_ui->DoubleMaxValue->setValue(stColumnToEdit->nMax);
                m_ui->DoubleDecimals->setValue(stColumnToEdit->nPrecision);
                break;
            case eColumnType::Text:
                m_ui->TextPropertiesWidget->setVisible(true);
                m_ui->TextMaxLength->setValue(stColumnToEdit->textMaxLength);
                break;
        }
        m_ui->OptionalCheckBox->setChecked(stColumnToEdit->bOptional);
    }

    if(m_ui->NameLineEdit->text().length() == 0)
        m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    QObject::connect(m_ui->TypeComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(TypeChanged(QString)));
    QObject::connect(m_ui->DoubleDecimals, SIGNAL(valueChanged(int)), this, SLOT(DecimalsPrecisionChanged(int)));
    QObject::connect(m_ui->NameLineEdit, SIGNAL(textEdited(QString)), this, SLOT(IsColumnValid()));
    QObject::connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(Validate()));
}

AddColumnDialog::~AddColumnDialog() {
    delete m_ui;
}

stColumn* AddColumnDialog::getColumn() {
    return &m_stColumn;
}

void AddColumnDialog::TypeChanged(QString sText) {
    m_ui->DoublePropertiesWidget->setVisible(false);
    m_ui->IntPropertiesWidget->setVisible(false);
    m_ui->TextPropertiesWidget->setVisible(false);
    if(QString::compare(sText, tr("Integer")) == 0) {
        m_ui->IntPropertiesWidget->setVisible(true);
    }
    else if(QString::compare(sText, tr("Text")) == 0) {
        m_ui->TextPropertiesWidget->setVisible(true);
    }
    else {
         m_ui->DoublePropertiesWidget->setVisible(true);
    }
}

void AddColumnDialog::DecimalsPrecisionChanged(int nPrecision) {
    m_ui->DoubleMinValue->setDecimals(nPrecision);
    m_ui->DoubleMaxValue->setDecimals(nPrecision);
}

void AddColumnDialog::IsColumnValid() {
    if(m_ui->NameLineEdit->text().length() > 0) {
         for(QString sColumn : sColumnNameList) {
             if(QString::compare(m_ui->NameLineEdit->text(), sColumn) == 0)
             {
                 m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
                 return;
             }
         }
        m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
    else
        m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

void AddColumnDialog::Validate() {
    m_stColumn.sName = m_ui->NameLineEdit->text();
    if(QString::compare(m_ui->TypeComboBox->currentText(), tr("Integer")) == 0) {
        m_stColumn.eType = eColumnType::Integer;
        m_stColumn.nMin = m_ui->IntMinValue->value();
        m_stColumn.nMax = m_ui->IntMaxValue->value();
    }
    else if(QString::compare(m_ui->TypeComboBox->currentText(), tr("Text")) == 0) {
        m_stColumn.eType = eColumnType::Text;
        m_stColumn.textMaxLength = m_ui->TextMaxLength->value();
    }
    else {
        m_stColumn.eType = eColumnType::Double;
        m_stColumn.nMin = m_ui->DoubleMinValue->value();
        m_stColumn.nMax = m_ui->DoubleMaxValue->value();
        m_stColumn.nPrecision = m_ui->DoubleDecimals->value();
    }
    m_stColumn.bOptional = m_ui->OptionalCheckBox->isChecked();
}
