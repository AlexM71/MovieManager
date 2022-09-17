#include "EditViewsDialog.h"
#include "ui_EditViewsDialog.h"

EditViewsDialog::EditViewsDialog(int* ID, Log* log, int* theme, QWidget* parent) : QDialog(parent) {
    m_ui = new Ui::EditViewsDialog;
    m_ui->setupUi(this);
    m_ID = ID;
    m_log = log;
    m_theme = theme;

    m_ui->tableWidget->setColumnHidden(0, true);

    QSqlQuery titleQuery;
    titleQuery.exec("SELECT Name FROM movies WHERE ID="+QString::number(*m_ID)+";");
    titleQuery.first();
    this->setWindowTitle(tr("Vues - ")+titleQuery.value(0).toString());

    QObject::connect(m_ui->tableWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customMenuRequested(QPoint)));

    fillTable();

}

EditViewsDialog::~EditViewsDialog() {
    delete m_ui;
}

void EditViewsDialog::fillTable() {

    int movieListRowCount = m_ui->tableWidget->rowCount();
    for(int i=movieListRowCount ; i >= 0 ; i--) {
        m_ui->tableWidget->removeRow(i);
    }

    QSqlQuery query;
    query.prepare("SELECT ID, ViewDate, ViewType FROM views WHERE ID_Movie="+QString::number(*m_ID)+" ORDER BY ViewDate DESC;");

    if(!query.exec()){
        m_log->append(tr("Erreur lors de la récupération dans la base de données, plus d'informations ci-dessous :\nCode d'erreur ")+query.lastError().nativeErrorCode()+tr(" : ")+query.lastError().text());
    }

    while(query.next()) {

         QTableWidgetItem* ID = new QTableWidgetItem();
         QTableWidgetItem* viewDate = new QTableWidgetItem();
         QTableWidgetItem* viewType = new QTableWidgetItem();

         ID->setText(query.value(0).toString());
         viewDate->setText(query.value(1).toString());
         viewType->setText(query.value(2).toString());

         m_ui->tableWidget->insertRow(m_ui->tableWidget->rowCount());

         m_ui->tableWidget->setItem(m_ui->tableWidget->rowCount()-1, 0, ID);
         m_ui->tableWidget->setItem(m_ui->tableWidget->rowCount()-1, 1, viewDate);
         m_ui->tableWidget->setItem(m_ui->tableWidget->rowCount()-1, 2, viewType);
    }
}


void EditViewsDialog::customMenuRequested(QPoint pos) {
    QMenu *menu = new QMenu(this);
    QAction* deleteAction = new QAction(tr("Supprimer"), this);
    QAction* editAction = new QAction(tr("Modifier"), this);


    if(*m_theme == Theme::Classic) {
        deleteAction->setIcon(QIcon(":/icons/Icons/remove.png"));
        editAction->setIcon(QIcon(":/icons/Icons/edit.png"));
    }
    else {
        deleteAction->setIcon(QIcon(":/icons/Icons/remove light.png"));
        editAction->setIcon(QIcon(":/icons/Icons/edit light.png"));
    }

    menu->addAction(editAction);
    menu->addAction(deleteAction);

    QObject::connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteView()));
    QObject::connect(editAction, SIGNAL(triggered()), this, SLOT(editView()));
    menu->popup(m_ui->tableWidget->viewport()->mapToGlobal(pos));
}

bool EditViewsDialog::edited() {
    return m_edited;
}

void EditViewsDialog::deleteView() {
    QSqlQuery deleteQuery;
    QString viewID = m_ui->tableWidget->item(m_ui->tableWidget->currentRow(),0)->text();

    deleteQuery.exec("DELETE FROM views WHERE ID=\""+viewID+"\";");
    fillTable();
    m_edited = true;
}

void EditViewsDialog::editView() {

    QString viewDate;
    QString viewType;
    QString viewID = m_ui->tableWidget->item(m_ui->tableWidget->currentRow(),0)->text();

    EditViewDialog* window = new EditViewDialog(m_ui->tableWidget, this);
    window->show();
    if(window->exec() == 1) {
        QSqlQuery editMovieQuery;

        viewType = window->getViewType();
        viewDate = window->getViewDate();

        if(window->isDateUnknown()) {
            viewDate = "?";
        }
        if(window->isTypeUnknown()) {
            viewType = "?";
        }
        editMovieQuery.exec("UPDATE views SET ViewDate=\""+viewDate+"\", ViewType=\""+viewType+"\" WHERE ID=\""+viewID+"\";");

        fillTable();
        m_edited = true;
    }
}
