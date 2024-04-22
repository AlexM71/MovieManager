#include "ChartsDialog.hpp"
#include "ui_ChartsDialog.h"

int ChartsDialog::instances = 0;

ChartsDialog::ChartsDialog(QWidget *parent) : QDialog(parent) {
    instances++;
    m_ui = new Ui::ChartsDialog;
    m_ui->setupUi(this);
    this->setWindowIcon(QIcon(":/assets/Assets/Icons/Dark/chart.png"));

    chartView = new QChartView();

    // Viewed By Movie Year
    movieYearSeries = new QBarSeries();
    movieYearSet = new QBarSet("");
    movieYearAxisX = new QBarCategoryAxis();
    movieYearAxisY = new QValueAxis();
    movieYearChart = new QChart();

    // Viewed By Year
    yearSeries = new QBarSeries();
    yearSet = new QBarSet("");
    yearAxisX = new QBarCategoryAxis();
    yearAxisY = new QValueAxis();
    yearChart = new QChart();

    // View Type
    viewTypeSeries = new QPieSeries();
    viewTypeChart = new QChart();

    // Initializing charts
    setViewedByMovieYearChart();
    setViewedByTypeChart();
    setViewedByYearChart();

    m_selectedChart = eChart::ViewedByMovieYear; // Set default chart
    setDisplayedChart((int)m_selectedChart);

    chartView->setRenderHint(QPainter::Antialiasing);

    chartLayout = new QHBoxLayout();

    m_ui->ChartWidget->setLayout(chartLayout);

    m_ui->ChartWidget->layout()->addWidget(chartView);

    QObject::connect(m_ui->ChartSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(setDisplayedChart(int)));
    //QObject::connect(movieYearSet, SIGNAL(hovered(bool,int)), this, SLOT());

}

ChartsDialog::~ChartsDialog() {
    instances--;
    delete m_ui;

    // Deletes movie year chart related elements
    delete movieYearSet;
    delete movieYearSeries;
    delete movieYearAxisX;
    delete movieYearAxisY;
    delete movieYearChart;

    // Deletes year chart related elements
    delete yearSet;
    delete yearSeries;
    delete yearAxisX;
    delete yearAxisY;
    delete yearChart;

    // View Type
    //delete viewTypeSeries; // TODO : memory leak here, find a way to make it work
    delete viewTypeChart;
    delete chartLayout;

    delete chartView;
}

int ChartsDialog::instancesCount() {
    return instances;
}

void ChartsDialog::setDisplayedChart(int nChart) {

    enum eChart eChart = (enum eChart)nChart;
    switch(eChart) {
        case eChart::ViewedByMovieYear:
            chartView->setMinimumWidth(35 * movieYearSet->count());
            chartView->setChart(movieYearChart);
            break;
        case eChart::ViewedByType:
            chartView->setMinimumWidth(0);
            chartView->setChart(viewTypeChart);
            break;
        case eChart::ViewedByYear:
            chartView->setMinimumWidth(35 * yearSet->count());
            chartView->setChart(yearChart);
            break;
    }
}

void ChartsDialog::setViewedByMovieYearChart() {

    sRequest = "SELECT ReleaseYear, count(*) FROM movies GROUP BY ReleaseYear;";

    QStringList categories;
    int maxValue = 0;

    if(!query.exec(sRequest)) {
        Common::LogDatabaseError(&query);
        return;
    }
    else {
        while(query.next()) {
            categories << query.value(0).toString();
            *movieYearSet << query.value(1).toInt();
            if(query.value(1).toInt() > maxValue)
                maxValue = query.value(1).toInt();
        }

        movieYearSeries->append(movieYearSet);

        movieYearChart->addSeries(movieYearSeries);
        movieYearChart->setAnimationOptions(QChart::SeriesAnimations);

        movieYearAxisX->append(categories);
        movieYearChart->addAxis(movieYearAxisX, Qt::AlignBottom);
        movieYearSeries->attachAxis(movieYearAxisX);
        movieYearAxisX->setLabelsAngle(90);

        movieYearAxisY->setRange(0,maxValue);
        movieYearChart->addAxis(movieYearAxisY, Qt::AlignLeft);
        movieYearSeries->attachAxis(movieYearAxisY);

        movieYearAxisY->setTickInterval(calculateChartTickInterval(maxValue));
        movieYearAxisY->setTickAnchor(0);
        movieYearAxisY->setTickType(QValueAxis::TicksDynamic);

        movieYearChart->legend()->hide();
    }
}

void ChartsDialog::setViewedByTypeChart() {

    sRequest = "SELECT ViewType, count(*) FROM views GROUP BY ViewType";

    if(!query.exec(sRequest)) {
        Common::LogDatabaseError(&query);
        return;
    }
    else {
        while(query.next()) {
            viewTypeSeries->append(Common::viewTypeToQString((enum eViewType)query.value(0).toInt()), query.value(1).toInt());
        }

        for(QPieSlice* slice : viewTypeSeries->slices()) {
            slice->setLabelVisible();
            slice->setLabelPosition(QPieSlice::LabelOutside);
        }

        viewTypeChart->addSeries(viewTypeSeries);
        viewTypeChart->legend()->hide();
    }
}

void ChartsDialog::setViewedByYearChart() {

    sRequest = "SELECT strftime('%Y', ViewDate) AS Year, count(*) FROM views WHERE ViewDate != '?' GROUP BY Year";
    QStringList categories;
    int maxValue = 0;

    if(!query.exec(sRequest)) {
        Common::LogDatabaseError(&query);
        return;
    }
    else {
        while(query.next()) {
            categories << query.value(0).toString();
            *yearSet << query.value(1).toInt();
            if(query.value(1).toInt() > maxValue)
                maxValue = query.value(1).toInt();
        }


        yearSeries->append(yearSet);

        yearChart->addSeries(yearSeries);
        yearChart->setAnimationOptions(QChart::SeriesAnimations);

        yearAxisX->append(categories);
        yearChart->addAxis(yearAxisX, Qt::AlignBottom);
        yearSeries->attachAxis(yearAxisX);

        yearAxisY->setRange(0,maxValue);
        yearChart->addAxis(yearAxisY, Qt::AlignLeft);
        yearSeries->attachAxis(yearAxisY);

        yearAxisY->setTickInterval(calculateChartTickInterval(maxValue));
        yearAxisY->setTickAnchor(0);
        yearAxisY->setTickType(QValueAxis::TicksDynamic);

        yearChart->legend()->hide();
    }
}

int ChartsDialog::calculateChartTickInterval(int maxValue) {
    if(maxValue < 0)
        return -1;
    else if(maxValue <= 10)
        return 2;
    else if(maxValue <= 20)
        return 5;
    else if(maxValue <= 50)
        return 10;
    else if(maxValue <= 100)
        return 20;
    else if(maxValue <= 200)
        return 50;
    else if(maxValue <= 500)
        return 100;
    else
        return 100;
}









