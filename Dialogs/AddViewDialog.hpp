#ifndef ADDVIEWDIALOG_HPP
#define ADDVIEWDIALOG_HPP

#include <QDialog>
#include <QSqlQuery>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QKeyEvent>
#include <QCompleter>

#include "Common.hpp"
#include "Tag.hpp"
#include "TagsScrollArea.hpp"
#include "CustomColumnLineEdit.hpp"

namespace Ui {
class AddViewDialog;
}

class AddViewDialog : public QDialog
{
    Q_OBJECT

    private:
        Ui::AddViewDialog* m_ui;
        QList<QString>* m_tags;
        QString m_posterPath;
        QList<CustomColumnLineEdit*>* m_customColumnInputList;
        TagsScrollArea* m_tagsScrollArea;
        QStringList m_sMovieList;
        QStringList m_sTagList;
        int m_customColumnCount;

        bool eventFilter(QObject *obj, QEvent *event);

    public:
        explicit AddViewDialog(QWidget *parent, int nMovieID = -1);
        ~AddViewDialog();

        QStringList GetMovieList();
        QStringList GetTagsList();

        bool IsSearchedMovieAnExistingMovie();
        QString getName();
        int getReleaseYear();
        enum eViewType getViewType();
        QDate getViewDate();
        QString getPosterPath();
        int getRating();
        QList<QString>* getTags();
        bool isDateUnknown();
        bool isTypeUnknown();
        CustomColumnLineEdit* getCustomColumnInputAt(int nIndex);
        QString GetSearchedMovieText();
        int getCustomColumnCount();

    public slots:
        void MovieSearchChanged();
        void toggleViewDateInput(int state);
        void toggleViewTypeInput(int state);
        void checkValid();
        void SelectPoster();
        void addTag();
        void CheckTag(QString sTag);

        void clickedTag(Tag* tag);
        void mouseEnteredTag(Tag* tag);
        void mouseLeftTag(Tag* tag);

};

#endif // ADDVIEWDIALOG_HPP
