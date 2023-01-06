#include "OptionsDialog.h"
#include "ui_OptionsDialog.h"

OptionsDialog::OptionsDialog(bool* matrixMode, enum eLanguage *language, enum eTheme *theme, bool* quickSearchCaseSensitive, QString* dateFormat, QWidget *parent) : QDialog(parent) {
    m_ui = new Ui::OptionsDialog;
    m_ui->setupUi(this);
    this->setWindowIcon(QIcon(":/assets/Assets/Icons/Dark/settings.png"));

    m_matrixMode = matrixMode;
    m_language = language;
    m_theme = theme;
    m_quickSearchCaseSensitive = quickSearchCaseSensitive;
    m_dateFormat = dateFormat;

    // Set current settings values
    m_ui->ThemeCombobox->setCurrentIndex(*m_theme);
    m_ui->LanguageCombobox->setCurrentIndex(*m_language);
    m_ui->MatrixModeCheckbox->setChecked(*m_matrixMode);
    m_ui->QuickSearchCaseCheckbox->setChecked(*m_quickSearchCaseSensitive);
    m_ui->DateFormatCombobox->setCurrentIndex(m_ui->DateFormatCombobox->findText(*dateFormat));
}

OptionsDialog::~OptionsDialog() {
    *m_matrixMode = m_ui->MatrixModeCheckbox->isChecked();
    *m_language = (enum eLanguage)m_ui->LanguageCombobox->currentIndex();
    *m_theme = (enum eTheme)m_ui->ThemeCombobox->currentIndex();
    *m_quickSearchCaseSensitive = m_ui->QuickSearchCaseCheckbox->isChecked();
    *m_dateFormat = m_ui->DateFormatCombobox->currentText();
    delete m_ui;
}
