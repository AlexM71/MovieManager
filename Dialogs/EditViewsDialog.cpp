#include "EditViewsDialog.hpp"
#include "ui_EditViewsDialog.h"

EditViewsDialog::EditViewsDialog(int* ID, QWidget* parent) : QDialog(parent) {
    m_ui = new Ui::EditViewsDialog;
    m_ui->setupUi(this);
    m_ID = ID;
    this->setWindowIcon(Common::GetIconAccordingToColorScheme(qApp->styleHints()->colorScheme(), "edit.png"));

    m_ui->tableWidget->setColumnHidden(0, true);

    QSqlQuery titleQuery;
    if(!titleQuery.exec("SELECT Name FROM movies WHERE ID="+QString::number(*m_ID)+";"))
        Common::LogDatabaseError(&titleQuery);
    titleQuery.first();
    this->setWindowTitle(tr("Views - %1").arg(titleQuery.value(0).toString()));

    QObject::connect(m_ui->tableWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customMenuRequested(QPoint)));
    QObject::connect(m_ui->tableWidget, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(editView()));

    fillTable();

}

EditViewsDialog::~EditViewsDialog() {
    delete m_ui;
}

int EditViewsDialog::GetViewsCount() {
    return m_ui->tableWidget->rowCount();
}

void EditViewsDialog::fillTable() {

    int movieListRowCount = m_ui->tableWidget->rowCount();
    for(int i=movieListRowCount ; i >= 0 ; i--) {
        m_ui->tableWidget->removeRow(i);
    }

    QSqlQuery query;
    query.prepare("SELECT ID, ViewDate, ViewType FROM views WHERE ID_Movie="+QString::number(*m_ID)+" ORDER BY ViewDate DESC;");

    if(!query.exec()){
        Common::LogDatabaseError(&query);
    }

    while(query.next()) {

         QTableWidgetItem* ID = new QTableWidgetItem();
         QTableWidgetItem* viewDate = new QTableWidgetItem();
         QTableWidgetItem* viewType = new QTableWidgetItem();

         ID->setText(query.value(0).toString());
         if(query.value(1).toString() == "?")
             viewDate->setText("?");
         else
            viewDate->setText(query.value(1).toDate().toString(Common::Settings->value("dateFormat").toString()));
         viewType->setText(Common::viewTypeToQString((enum eViewType)query.value(2).toInt()));

         m_ui->tableWidget->insertRow(m_ui->tableWidget->rowCount());

         m_ui->tableWidget->setItem(m_ui->tableWidget->rowCount()-1, 0, ID);
         m_ui->tableWidget->setItem(m_ui->tableWidget->rowCount()-1, 1, viewDate);
         m_ui->tableWidget->setItem(m_ui->tableWidget->rowCount()-1, 2, viewType);
    }
}


void EditViewsDialog::customMenuRequested(QPoint pos) {
    QMenu *menu = new QMenu(this);

    QAction* editAction = new QAction(tr("Edit"), this);
    editAction->setIcon(Common::GetIconAccordingToTheme((enum eTheme)Common::Settings->value("theme").toInt(), "edit.png"));

    QAction* deleteAction = new QAction(tr("Delete"), this);
    deleteAction->setIcon(Common::GetIconAccordingToTheme((enum eTheme)Common::Settings->value("theme").toInt(), "delete.png"));

    menu->addAction(editAction);
    menu->addAction(deleteAction);

    QObject::connect(editAction, SIGNAL(triggered()), this, SLOT(editView()));
    QObject::connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteView()));

    menu->popup(m_ui->tableWidget->viewport()->mapToGlobal(pos));
}

bool EditViewsDialog::edited() {
    return m_edited;
}

void EditViewsDialog::deleteView() {
    QSqlQuery deleteQuery;
    QString viewID = m_ui->tableWidget->item(m_ui->tableWidget->currentRow(),0)->text();

    if(!deleteQuery.exec("DELETE FROM views WHERE ID=\""+viewID+"\";"))
         Common::LogDatabaseError(&deleteQuery);
    fillTable();
    m_edited = true;
}

void EditViewsDialog::editView() {

    QString viewDate;
    enum eViewType viewType;
    QString viewID = m_ui->tableWidget->item(m_ui->tableWidget->currentRow(),0)->text();

    EditViewDialog* window = new EditViewDialog(m_ui->tableWidget, this);
    window->show();
    if(window->exec() == 1) {
        QSqlQuery editMovieQuery;

        viewType = window->getViewType();
        viewDate = window->getViewDate().toString("yyyy-MM-dd");

        if(window->isDateUnknown()) {
            viewDate = "?";
        }
        if(window->isTypeUnknown()) {
            viewType = eViewType::Unknown;
        }
        if(!editMovieQuery.exec("UPDATE views SET ViewDate=\""+viewDate+"\", ViewType=\""+QString::number((int)viewType)+"\" WHERE ID=\""+viewID+"\";"))
            Common::LogDatabaseError(&editMovieQuery);

        fillTable();
        m_edited = true;
    }
}
