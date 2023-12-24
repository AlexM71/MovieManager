#ifndef EDITVIEWSDIALOG_H
#define EDITVIEWSDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QSqlQuery>
#include <QSqlError>
#include <QPushButton>
#include <QMenu>

#include "Enums.h"
#include "EditViewDialog.h"

namespace Ui {
class EditViewsDialog;
}

class EditViewsDialog : public QDialog
{
    Q_OBJECT

    private:
        Ui::EditViewsDialog* m_ui;
        int* m_ID;
        bool m_edited = false;

    public:
        explicit EditViewsDialog(int* ID, QWidget *parent = nullptr);
        ~EditViewsDialog();

        void fillTable();
        bool edited();
        int GetViewsCount();

    public slots:
        void customMenuRequested(QPoint pos);
        void deleteView();
        void editView();


};

#endif // EDITVIEWSDIALOG_H
