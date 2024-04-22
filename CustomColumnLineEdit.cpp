#include "CustomColumnLineEdit.hpp"


CustomColumnLineEdit::CustomColumnLineEdit(enum eColumnType eType)
{
    QObject::connect(this, SIGNAL(textEdited(QString)), this, SLOT(CheckForOutline(QString)));
    QObject::connect(this, SIGNAL(editingFinished()), this, SLOT(Check()));

    m_eType = eType;

    // Default values
    m_sLabel = "";
    m_fMin = 0;
    m_fMax = 0;
    m_nPrecision = 0;
    m_nTextMaxLength = 0;
    m_bOptional = false;
}

CustomColumnLineEdit::~CustomColumnLineEdit()
{
}

void CustomColumnLineEdit::setLabel(QString sLabel)
{
    m_sLabel = sLabel;
}

QString CustomColumnLineEdit::getLabel()
{
    return m_sLabel;
}

void CustomColumnLineEdit::setType(enum eColumnType eType)
{
    m_eType = eType;
}

enum eColumnType CustomColumnLineEdit::getType()
{
    return m_eType;
}

double CustomColumnLineEdit::getMin() const
{
    return m_fMin;
}

void CustomColumnLineEdit::setMin(double fMin)
{
    m_fMin = fMin;
}

double CustomColumnLineEdit::getMax() const
{
    return m_fMax;
}

void CustomColumnLineEdit::setMax(double fMax)
{
    m_fMax = fMax;
}

int CustomColumnLineEdit::getPrecision() const
{
    return m_nPrecision;
}

void CustomColumnLineEdit::setPrecision(int nPrecision)
{
    m_nPrecision = nPrecision;
}

int CustomColumnLineEdit::getTextMaxLength() const
{
    return m_nTextMaxLength;
}

void CustomColumnLineEdit::setTextMaxLength(int nTextMaxLength)
{
    m_nTextMaxLength = nTextMaxLength;
}

bool CustomColumnLineEdit::IsOptional() const
{
    return m_bOptional;
}

void CustomColumnLineEdit::setOptional(bool bOptional)
{
    m_bOptional = bOptional;
}

void CustomColumnLineEdit::Check()
{
    QString sText = this->text();
    bool bTextValid = true;
    if(sText.length() != 0)
    {
        switch(m_eType)
        {
        case eColumnType::Text:
            bTextValid = CheckText(sText);
            break;
        case eColumnType::Integer:
            bTextValid = CheckInteger(sText);
            break;
        case eColumnType::Double:
            bTextValid = CheckDouble(sText);
            break;
        }
    }
    if(bTextValid == false)
        this->setText(m_sLastValidText);
    else
        m_sLastValidText = sText;

    CheckForOutline(this->text());
}

void CustomColumnLineEdit::CheckForOutline(QString sText)
{
    bool bTextValid = true;
    if(sText.length() != 0)
    {
        switch(m_eType)
        {
        case eColumnType::Text:
            bTextValid = CheckText(sText);
            break;
        case eColumnType::Integer:
            bTextValid = CheckInteger(sText);
            break;
        case eColumnType::Double:
            bTextValid = CheckDouble(sText);
            break;
        }
    }
    if(bTextValid == false)
        this->setStyleSheet("border: 1px solid #f00;");
    else
        this->setStyleSheet("");
}

bool CustomColumnLineEdit::CheckText(QString sText)
{
    bool bValid = true;
    if(sText.length() > m_nTextMaxLength)
        bValid = false;

    return bValid;
}

bool CustomColumnLineEdit::CheckInteger(QString sText)
{
    bool bValid = false;

    bool bConversionOK;
    int nValue = sText.toInt(&bConversionOK);
    if(bConversionOK == true)
    {
        if(nValue >= m_fMin && nValue <= m_fMax)
            bValid = true;
    }

    return bValid;
}

bool CustomColumnLineEdit::CheckDouble(QString sText)
{
    bool bValid = false;
    bool bConversionOK;
    double fValue = sText.toDouble(&bConversionOK);

    if(bConversionOK == true)
    {
        if(fValue >= m_fMin && fValue <= m_fMax)
            bValid = true;

        QChar cDecimalSeparator = '.'; // May cause issues
        int nDecimalsCount;
        if(sText.indexOf(cDecimalSeparator) == -1)
            nDecimalsCount = 0;
        else
        {
            QString sDecimals = sText.last(sText.length() - sText.indexOf(cDecimalSeparator) - 1);
            nDecimalsCount = sDecimals.length();
        }

        if(nDecimalsCount > m_nPrecision)
            bValid = false;
    }

    return bValid;
}
