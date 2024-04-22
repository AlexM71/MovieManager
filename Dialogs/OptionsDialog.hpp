#ifndef OPTIONSDIALOG_HPP
#define OPTIONSDIALOG_HPP

#include <QDialog>
#include <QSignalMapper>

#include "Enums.hpp"
#include "Common.hpp"
#include "AddColumnDialog.hpp"

namespace Ui {
    class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

    private:
        Ui::OptionsDialog* m_ui;
        QSignalMapper* m_editColumnSignalMapper;
        QSignalMapper* m_deleteColumnSignalMapper;
        QWidget* pPreviousWidget; // Used for tabulation order

    public:
        explicit OptionsDialog(QWidget* parent = nullptr);
        ~OptionsDialog();

        void InsertColumn(struct stColumn* stColumnToInsert, int nRow);
        void InsertColumnQt(QString sName, enum eColumnType eType, int nRow);
        void InsertColumnDB(struct stColumn* stColumnToInsert);

        void RemoveColumnQt(int nRow);
        void RemoveColumnDB(QString sName);

    public slots:
        void AddColumn();
        void EditColumn(int nRow);
        void RemoveColumn(int nRow);
};

#endif // OPTIONSDIALOG_HPP
