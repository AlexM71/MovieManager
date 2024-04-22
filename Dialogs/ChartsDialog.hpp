#ifndef CHARTSDIALOG_HPP
#define CHARTSDIALOG_HPP

#include <QDialog>
#include <QSqlQuery>
#include <QPainter>

#include <QChart>
#include <QBarSet>
#include <QBarSeries>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QChartView>
#include <QPieSeries>

#include "Enums.hpp"
#include "Common.hpp"
#include "qsqlerror.h"

namespace Ui {
class ChartsDialog;
}

class ChartsDialog : public QDialog {
    Q_OBJECT

    private:
        Ui::ChartsDialog* m_ui;
        static int instances;
        enum eChart m_selectedChart;

        QChartView* chartView; // Widget containing the complete chart
        QString sRequest;
        QSqlQuery query;

        // Viewed By Movie Year
        QBarSeries* movieYearSeries;
        QBarSet* movieYearSet;
        QBarCategoryAxis* movieYearAxisX;
        QValueAxis* movieYearAxisY;
        QChart* movieYearChart;

        // Viewed By Year
        QBarSeries* yearSeries;
        QBarSet* yearSet;
        QBarCategoryAxis* yearAxisX;
        QValueAxis* yearAxisY;
        QChart* yearChart;

        // View Type
        QPieSeries* viewTypeSeries;
        QChart* viewTypeChart;

        QHBoxLayout* chartLayout;

    public:
        explicit ChartsDialog(QWidget* parent = nullptr);
        static int instancesCount();
        ~ChartsDialog();

        void setViewedByMovieYearChart();
        void setViewedByTypeChart();
        void setViewedByYearChart();
        void setHeatmapChart();
        int calculateChartTickInterval(int maxValue);

    public slots:
        void setDisplayedChart(int nChart);

};

#endif // CHARTSDIALOG_HPP
