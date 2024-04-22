#ifndef CUSTOMCOLUMNLINEEDIT_HPP
#define CUSTOMCOLUMNLINEEDIT_HPP

#include <QLineEdit>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

#include "Enums.hpp"

class CustomColumnLineEdit : public QLineEdit
{
    Q_OBJECT

    private:
        enum eColumnType m_eType;
        QString m_sLabel;
        QString m_sLastValidText;
        double m_fMin;
        double m_fMax;
        int m_nPrecision;
        int m_nTextMaxLength;
        bool m_bOptional;

        bool CheckText(QString sText);
        bool CheckInteger(QString sText);
        bool CheckDouble(QString sText);
    public:
        CustomColumnLineEdit(enum eColumnType eType);
        ~CustomColumnLineEdit();

        // Getters/Setters
        void setLabel(QString sLabel);
        QString getLabel();
        void setType(enum eColumnType eType);
        enum eColumnType getType();
        double getMin() const;
        void setMin(double fMin);
        double getMax() const;
        void setMax(double fMax);
        int getPrecision() const;
        void setPrecision(int nPrecision);
        int getTextMaxLength() const;
        void setTextMaxLength(int nNewTextMaxLength);
        bool IsOptional() const;
        void setOptional(bool bOptional);

    public slots:
        void Check();
        void CheckForOutline(QString sText);
};

#endif // CUSTOMCOLUMNLINEEDIT_HPP
