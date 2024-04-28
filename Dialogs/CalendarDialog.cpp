#include "CalendarDialog.hpp"
#include "ui_CalendarDialog.h"

int CalendarDialog::instances = 0;

CalendarDialog::CalendarDialog(QWidget *parent) : QDialog(parent) {
    instances++;
    m_ui = new Ui::CalendarDialog;
    m_ui->setupUi(this);
    this->setWindowIcon(Common::GetIconAccordingToColorScheme(qApp->styleHints()->colorScheme(), "calendar.png"));

    m_labels = new QList<QLabel*>;

    m_selectedDate = QDate::currentDate();

    setDisplay();
    setData();

    QObject::connect(m_ui->PreviousPushButton, SIGNAL(clicked()), this, SLOT(previousButtonClicked()));
    QObject::connect(m_ui->NextPushButton, SIGNAL(clicked()), this, SLOT(nextButtonClicked()));
}

CalendarDialog::~CalendarDialog() {
    instances--;

    deleteLabels();
    delete m_labels;

    delete m_ui;
}

void CalendarDialog::setDisplay() {

    Common::clearLayout(m_ui->CalendarLayout);

    m_ui->CurrentMonthLabel->setText(m_monthLabel.at(m_selectedDate.month()-1) + " " + QString::number(m_selectedDate.year()));

    int pos;
    int offset = getOffset(); //Used to indicate where the 1st day of the month will be, value between 0 and 6

    // Creates a QWidget for each day in the month
    for(int i = 0 ; i < m_selectedDate.daysInMonth() ; i++) {
        pos = i+offset;
        QWidget* dayWidget = new QWidget();
        QVBoxLayout* dayLayout = new QVBoxLayout();
        dayWidget->setLayout(dayLayout);
        QLabel* dayLabel = new QLabel(QString::number(i+1));

        QFont f = dayLabel->font();
        f.setBold(true);
        dayLabel->setFont(f);

        dayLabel->setAlignment(Qt::AlignHCenter);
        dayLayout->addWidget(dayLabel);
        m_ui->CalendarLayout->addWidget(dayWidget, pos/7, pos%7);

    }
}

void CalendarDialog::setData() {
    QString year = QString::number(m_selectedDate.year());
    QString month;
    if(m_selectedDate.month() < 10) {
        month = "0"+QString::number(m_selectedDate.month());
    }
    else {
        month = QString::number(m_selectedDate.month());
    }


    QSqlQuery selectedDateViews;
    if(!selectedDateViews.exec("SELECT ID, ID_Movie, ViewDate, ViewType FROM views WHERE ViewDate BETWEEN '"+year+"-"+month+"-01' AND '"+year+"-"+month+"-31'"))
        Common::LogDatabaseError(&selectedDateViews);
    while(selectedDateViews.next()) {
        int viewDay = selectedDateViews.value(2).toString().remove(0,8).toInt();
        QSqlQuery movieInfos;
        if(!movieInfos.exec("SELECT Name, ReleaseYear FROM movies WHERE ID=\""+selectedDateViews.value(1).toString()+"\""))
            Common::LogDatabaseError(&movieInfos);
        movieInfos.first();
        QLabel* viewLabel = new QLabel(movieInfos.value(0).toString() + " - " + movieInfos.value(1).toString());
        m_labels->append(viewLabel);
        viewLabel->setMaximumWidth((this->width()/7) - (9*2));
        viewLabel->setToolTip(movieInfos.value(0).toString() + " - " + movieInfos.value(1).toString());
        viewLabel->setFixedHeight(24);
        viewLabel->setProperty("class", "Calendar-view-class");

        viewLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        m_ui->CalendarLayout->itemAt(viewDay-1)->widget()->layout()->addWidget(viewLabel);
    }

    for(int i = 0 ; i < m_selectedDate.daysInMonth() ; i++) {
        QVBoxLayout* l = (QVBoxLayout*)m_ui->CalendarLayout->itemAt(i)->widget()->layout();
        l->insertStretch(l->count());
    }


}

int CalendarDialog::instancesCount() {
    return instances;
}

int CalendarDialog::getOffset() {
    bool isLeapYear = false;

    int offsetYearDebut = -1;
    int currentYearMod = m_selectedDate.year()%28;

    if(m_selectedDate.year()%4 == 0) {
        isLeapYear = true;
    }

    switch(currentYearMod) {
        case 2:
        case 8:
        case 13:
        case 19:
            offsetYearDebut = 0; //Year begun on monday
            break;
        case 3:
        case 14:
        case 20:
        case 25:
            offsetYearDebut = 1;
            break;
        case 4:
        case 9:
        case 15:
        case 26:
            offsetYearDebut = 2;
            break;
        case 10:
        case 16:
        case 21:
        case 27:
            offsetYearDebut = 3;
            break;
        case 0:
        case 5:
        case 11:
        case 22:
            offsetYearDebut = 4;
            break;
        case 6:
        case 12:
        case 17:
        case 23:
            offsetYearDebut = 5;
            break;
        case 1:
        case 7:
        case 18:
        case 24:
            offsetYearDebut = 6; //Year begun on sunday
            break;
        default:
            Common::Log->append(tr("Unable to determine the first day of the current year"), eLog::Error);
            return -1;
    }
    int currentOffset = offsetYearDebut;
    int nextOffset = -1;
    for(int i = 0 ; i < m_selectedDate.month()-1 ; i++) {
        if(i==1 && isLeapYear == true) {
            nextOffset = (currentOffset+m_daysPerMonth[i]+1) % 7;
        }
        else {
            nextOffset = (currentOffset+m_daysPerMonth[i]) % 7;
        }
        currentOffset = nextOffset;
    }

    return currentOffset;
}

void CalendarDialog::previousButtonClicked() {
    int year = m_selectedDate.year();
    int month = m_selectedDate.month();
    if(month-1 == 0) {
        year--;
        month = 12;
    }
    else {
        month--;
    }
    m_selectedDate.setDate(year, month, 1);

    deleteLabels();
    setDisplay();
    setData();
}

void CalendarDialog::nextButtonClicked() {
    int year = m_selectedDate.year();
    int month = m_selectedDate.month();
    if(month+1 == 13) {
        year++;
        month = 1;
    }
    else {
        month++;
    }
    m_selectedDate.setDate(year, month, 1);

    deleteLabels();
    setDisplay();
    setData();
}

void CalendarDialog::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);
    for(int nLabel = 0; nLabel < m_labels->size(); nLabel++)
    {
        m_labels->at(nLabel)->setMaximumWidth((this->width()/7) - (9*2));
    }
}

void CalendarDialog::deleteLabels()
{
    for(int nLabel = m_labels->size() - 1; nLabel >= 0; nLabel--)
    {
        delete m_labels->at(nLabel);
    }

    m_labels->clear();
}
