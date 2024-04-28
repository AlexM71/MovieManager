#include "FiltersDialog.hpp"
#include "ui_FiltersDialog.h"

FiltersDialog::FiltersDialog(QString sRequest, QWidget *parent) : QDialog(parent) {
    m_ui = new Ui::FiltersDialog;
    m_ui->setupUi(this);
    this->setWindowIcon(Common::GetIconAccordingToColorScheme(qApp->styleHints()->colorScheme(), "search.png"));

    CustomColumnLineEdit* LowReleaseYear = new CustomColumnLineEdit(eColumnType::Integer);
    LowReleaseYear->setLabel("ReleaseYear");
    LowReleaseYear->setMin(1870);
    LowReleaseYear->setMax(QDate::currentDate().year());
    m_ui->FormLayout->addWidget(LowReleaseYear, 1, 2);

    CustomColumnLineEdit* HighReleaseYear = new CustomColumnLineEdit(eColumnType::Integer);
    HighReleaseYear->setLabel("ReleaseYear");
    HighReleaseYear->setMin(1870);
    HighReleaseYear->setMax(QDate::currentDate().year());
    m_ui->FormLayout->addWidget(HighReleaseYear, 1, 4);

    CustomColumnLineEdit* LowRating = new CustomColumnLineEdit(eColumnType::Integer);
    LowRating->setLabel("Rating");
    LowRating->setMax(10);
    m_ui->FormLayout->addWidget(LowRating, 2, 2);

    CustomColumnLineEdit* HighRating = new CustomColumnLineEdit(eColumnType::Integer);
    HighRating->setLabel("Rating");
    HighRating->setMax(10);
    m_ui->FormLayout->addWidget(HighRating, 2, 4);

    FilterComboBox* ReleaseYearCombobox = new FilterComboBox(eColumnType::Integer);
    ReleaseYearCombobox->insertItem((int)eCombobox::None, "");
    ReleaseYearCombobox->insertItem((int)eCombobox::And, tr("AND"));
    ReleaseYearCombobox->insertItem((int)eCombobox::Or, tr("OR"));
    m_ui->FormLayout->addWidget(ReleaseYearCombobox, 1, 0);
    ReleaseYearCombobox->AddRelatedInput(LowReleaseYear);
    ReleaseYearCombobox->AddRelatedInput(HighReleaseYear);
    QObject::connect(ReleaseYearCombobox, SIGNAL(currentIndexChanged(int)), ReleaseYearCombobox, SLOT(ToggleWidgetState(int)));
    ReleaseYearCombobox->ToggleWidgetState(0);
    m_filters.append(ReleaseYearCombobox);

    FilterComboBox* RatingCombobox = new FilterComboBox(eColumnType::Integer);
    RatingCombobox->insertItem((int)eCombobox::None, "");
    RatingCombobox->insertItem((int)eCombobox::And, tr("AND"));
    RatingCombobox->insertItem((int)eCombobox::Or, tr("OR"));
    m_ui->FormLayout->addWidget(RatingCombobox, 2, 0);
    RatingCombobox->AddRelatedInput(LowRating);
    RatingCombobox->AddRelatedInput(HighRating);
    QObject::connect(RatingCombobox, SIGNAL(currentIndexChanged(int)), RatingCombobox, SLOT(ToggleWidgetState(int)));
    RatingCombobox->ToggleWidgetState(0);
    m_filters.append(RatingCombobox);


    QWidget::setTabOrder(m_ui->NameInput, ReleaseYearCombobox);
    QWidget::setTabOrder(ReleaseYearCombobox, LowReleaseYear);
    QWidget::setTabOrder(LowReleaseYear, HighReleaseYear);
    QWidget::setTabOrder(HighReleaseYear, RatingCombobox);
    QWidget::setTabOrder(RatingCombobox, LowRating);
    QWidget::setTabOrder(LowRating, HighRating);

    QWidget* pPreviousWidget = HighRating; // Used for tabulation order

    //m_ui->FormLayout
    QSqlQuery customColumnsQuery;
    if(!customColumnsQuery.exec("SELECT Name, Type, Min, Max, Precision, TextMaxLength, Optional FROM columns;"))
        Common::LogDatabaseError(&customColumnsQuery);
    int nColumnIndex = 0;
    while(customColumnsQuery.next()) {
        FilterComboBox* combobox = new FilterComboBox((enum eColumnType)customColumnsQuery.value(1).toInt());
        combobox->insertItem((int)eCombobox::None, "");
        combobox->insertItem((int)eCombobox::And, tr("AND"));
        combobox->insertItem((int)eCombobox::Or, tr("OR"));
        m_ui->FormLayout->addWidget(combobox, 3 + nColumnIndex, 0);

        QWidget::setTabOrder(pPreviousWidget, combobox);

        m_filters.append(combobox);

        QObject::connect(combobox, SIGNAL(currentIndexChanged(int)), combobox, SLOT(ToggleWidgetState(int)));

        QLabel* columnLabel = new QLabel(customColumnsQuery.value(0).toString());
        if(customColumnsQuery.value(1).toInt() == 0 || customColumnsQuery.value(1).toInt() == 1)
            columnLabel->setText(tr("%1 between").arg(customColumnsQuery.value(0).toString()));

        m_ui->FormLayout->addWidget(columnLabel, 3 + nColumnIndex, 1);

        if(customColumnsQuery.value(1).toInt() == (int)eColumnType::Integer
        || customColumnsQuery.value(1).toInt() == (int)eColumnType::Double)
        {
            CustomColumnLineEdit* LowInput = new CustomColumnLineEdit((enum eColumnType)customColumnsQuery.value(1).toInt());
            LowInput->setLabel(customColumnsQuery.value(0).toString());
            LowInput->setMin(customColumnsQuery.value(2).toDouble());
            LowInput->setMax(customColumnsQuery.value(3).toDouble());
            LowInput->setPrecision(customColumnsQuery.value(4).toInt());
            LowInput->setTextMaxLength(customColumnsQuery.value(5).toInt());
            LowInput->setOptional(customColumnsQuery.value(6).toBool());

            CustomColumnLineEdit* HighInput = new CustomColumnLineEdit((enum eColumnType)customColumnsQuery.value(1).toInt());
            HighInput->setLabel(customColumnsQuery.value(0).toString());
            HighInput->setMin(customColumnsQuery.value(2).toDouble());
            HighInput->setMax(customColumnsQuery.value(3).toDouble());
            HighInput->setPrecision(customColumnsQuery.value(4).toInt());
            HighInput->setTextMaxLength(customColumnsQuery.value(5).toInt());
            HighInput->setOptional(customColumnsQuery.value(6).toBool());

            combobox->AddRelatedInput(LowInput);
            combobox->AddRelatedInput(HighInput);

            m_ui->FormLayout->addWidget(LowInput, 3 + nColumnIndex, 2);
            m_ui->FormLayout->addWidget(HighInput, 3 + nColumnIndex, 4);

            QWidget::setTabOrder(combobox, LowInput);
            QWidget::setTabOrder(LowInput, HighInput);
            pPreviousWidget = HighInput;

            QLabel* andLabel = new QLabel(tr("and"));
            andLabel->setAlignment(Qt::AlignCenter);
            m_ui->FormLayout->addWidget(andLabel, 3 + nColumnIndex, 3);
        }
        else if(customColumnsQuery.value(1).toInt() == (int)eColumnType::Text)
        {
            CustomColumnLineEdit* input = new CustomColumnLineEdit((enum eColumnType)customColumnsQuery.value(1).toInt());
            input->setLabel(customColumnsQuery.value(0).toString());
            input->setMin(customColumnsQuery.value(2).toDouble());
            input->setMax(customColumnsQuery.value(3).toDouble());
            input->setPrecision(customColumnsQuery.value(4).toInt());
            input->setTextMaxLength(customColumnsQuery.value(5).toInt());
            input->setOptional(customColumnsQuery.value(6).toBool());

            combobox->AddRelatedInput(input);


            pPreviousWidget = input;

            m_ui->FormLayout->addWidget(input, 3 + nColumnIndex, 2, 1, 3);

            QWidget::setTabOrder(combobox, input);
        }
        nColumnIndex++;

        combobox->ToggleWidgetState(0);
    }
    this->adjustSize();

    SQLRequestToFilters(sRequest);
}

FiltersDialog::~FiltersDialog() {
    delete m_ui;
}

QString FiltersDialog::FiltersToSQLRequest()
{
    QString sRequest = QString(" WHERE \"Name\" LIKE \"%%1%\" ").arg(m_ui->NameInput->text());

    for(int nColumn = 0; nColumn < m_filters.length(); nColumn++)
    {
        FilterComboBox* combobox = m_filters[nColumn];
        if(combobox->currentIndex() == (int)eCombobox::None)
            continue;
        else if(combobox->currentIndex() == (int)eCombobox::And)
            sRequest += "AND ";
        else if(combobox->currentIndex() == (int)eCombobox::Or)
            sRequest += "OR ";

        if(combobox->getColumnType() == eColumnType::Integer || combobox->getColumnType() == eColumnType::Double)
        {
            QString sColumnName = combobox->getWidgetAt(0)->getLabel();
            QString sColumnLowValue = combobox->getWidgetAt(0)->text();
            QString sColumnHighValue = combobox->getWidgetAt(1)->text();

            if(sColumnLowValue == "")
                sColumnLowValue = QString::number(combobox->getWidgetAt(0)->getMin());
            if(sColumnHighValue == "")
                sColumnHighValue = QString::number(combobox->getWidgetAt(1)->getMax());

            sRequest += QString("\"%1\" BETWEEN %2 AND %3 ").arg(sColumnName, sColumnLowValue, sColumnHighValue);
        }
        else if(combobox->getColumnType() == eColumnType::Text)
        {
            QString sColumnName = combobox->getWidgetAt(0)->getLabel();
            QString sColumnValue = combobox->getWidgetAt(0)->text();

            //sRequest += sColumnName + " LIKE '%" + sColumnValue + "%'";
            sRequest += QString("\"%1\" LIKE \"%%2%\" ").arg(sColumnName, sColumnValue);
        }
    }

    sRequest += ";";

    return sRequest;
}

void FiltersDialog::SQLRequestToFilters(QString sRequest)
{
    QStringList sRequestList = sRequest.split("\"");

    // Fetch Name filter
    if(sRequestList.contains("Name") == true)
    {
        QString sName = sRequestList.at(sRequestList.indexOf("Name") + 2);
        sName.chop(1);
        sName = sName.remove(0,1);
        m_ui->NameInput->setText(sName);
    }


    enum eCombobox eCombo;
    QString sText;

    // Fetch filter for all other columns
    for(int nColumn = 0; nColumn < m_filters.length(); nColumn++)
    {
        FilterComboBox* combobox = m_filters.at(nColumn);

        if(sRequestList.contains(combobox->getWidgetAt(0)->getLabel()) == false)
        {
            combobox->setCurrentIndex((int)eCombobox::None);
        }
        else
        {
            int nColumnIndex = sRequestList.indexOf(combobox->getWidgetAt(0)->getLabel());
            QString sFilterType = sRequestList.at(nColumnIndex-1);
            if(sFilterType.endsWith("AND "))
                eCombo = eCombobox::And;
            else if(sFilterType.endsWith("OR "))
                eCombo = eCombobox::Or;

            combobox->setCurrentIndex((int)eCombo);

            if(combobox->getColumnType() == eColumnType::Integer || combobox->getColumnType() == eColumnType::Double)
            {
                CustomColumnLineEdit* lowInput = combobox->getWidgetAt(0);
                CustomColumnLineEdit* highInput = combobox->getWidgetAt(1);

                QString sText = sRequestList.at(nColumnIndex + 1);
                QStringList sListText = sText.split(" ");

                lowInput->setText(sListText.at(2));
                highInput->setText(sListText.at(4));
            }
            else if(combobox->getColumnType() == eColumnType::Text)
            {
                CustomColumnLineEdit* input = combobox->getWidgetAt(0);

                sText = sRequestList.at(nColumnIndex + 2);
                sText.chop(1);
                sText = sText.remove(0,1);

                input->setText(sText);
            }
        }
    }
}

FilterComboBox::FilterComboBox(enum eColumnType columnType)
{
    m_columnType = columnType;
}

enum eColumnType FilterComboBox::getColumnType()
{
    return m_columnType;
}

CustomColumnLineEdit* FilterComboBox::getWidgetAt(int nIndex)
{
    if(nIndex < m_relatedWidgets.length())
        return m_relatedWidgets.at(nIndex);
    else
        return nullptr;
}

void FilterComboBox::AddRelatedInput(CustomColumnLineEdit* input)
{
    m_relatedWidgets.append(input);
}

void FilterComboBox::ToggleWidgetState(int comboboxIndex)
{
    if(comboboxIndex == 0)
        for(int nWidget = 0; nWidget < m_relatedWidgets.length(); nWidget++)
            m_relatedWidgets.at(nWidget)->setEnabled(false);
    else
        for(int nWidget = 0; nWidget < m_relatedWidgets.length(); nWidget++)
            m_relatedWidgets.at(nWidget)->setEnabled(true);
}
