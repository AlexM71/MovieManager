#ifndef EDITVIEWDIALOG_HPP
#define EDITVIEWDIALOG_HPP

#include <QDialog>
#include <QDate>
#include <QTableWidget>

#include "Common.hpp"
#include "Enums.hpp"

namespace Ui {
    class EditViewDialog;
}

class EditViewDialog : public QDialog {
    Q_OBJECT

    private:
        Ui::EditViewDialog* m_ui;

    public:
        explicit EditViewDialog(QTableWidget* table, QWidget* parent = nullptr);
        ~EditViewDialog();

        QDate getViewDate();
        enum eViewType getViewType();
        bool isDateUnknown();
        bool isTypeUnknown();

   public slots:
        void toggleViewDateInput(int state);
        void toggleViewTypeInput(int state);

};

#endif // EDITVIEWDIALOG_HPP
