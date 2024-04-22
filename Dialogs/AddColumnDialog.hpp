#ifndef ADDCOLUMNDIALOG_HPP
#define ADDCOLUMNDIALOG_HPP

#include <QDialog>
#include <QPushButton>
#include <QStringList>
#include "Structures.hpp"
#include "Common.hpp"

namespace Ui {
    class AddColumnDialog;
}

class AddColumnDialog : public QDialog
{
    Q_OBJECT

    private:
        Ui::AddColumnDialog* m_ui;
        struct stColumn m_stColumn;
        QStringList sColumnNameList;

    public:
        explicit AddColumnDialog(QWidget *parent = nullptr, struct stColumn* stColumnToEdit = nullptr);
        ~AddColumnDialog();
        stColumn* getColumn();

    public slots:
        void TypeChanged(QString sText);
        void DecimalsPrecisionChanged(int nPrecision);
        void IsColumnValid();
        void Validate();
};

#endif // ADDCOLUMNDIALOG_HPP
