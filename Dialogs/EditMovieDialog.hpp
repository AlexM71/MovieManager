#ifndef EDITMOVIEDIALOG_HPP
#define EDITMOVIEDIALOG_HPP

#include <QDialog>
#include <QSqlQuery>
#include <QDate>
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
class EditMovieDialog;
}

class EditMovieDialog : public QDialog
{
    Q_OBJECT

    private:
        Ui::EditMovieDialog* m_ui;
        QString* m_ID;
        bool m_edited = false;
        bool m_newPoster = false;
        QString m_posterPath;
        QList<QString>* m_tags;
        QList<CustomColumnLineEdit*>* m_customColumnInputList;
        TagsScrollArea* m_tagsScrollArea;
        int m_customColumnCount;
        QStringList m_sTagList;

        bool eventFilter(QObject *obj, QEvent *event);

    public:
        explicit EditMovieDialog(QString ID, QWidget *parent = nullptr);
        ~EditMovieDialog();

        bool edited();
        bool newPoster();
        QString getMovieName();
        QString getReleaseYear();
        QString getPosterPath();
        int getRating();
        QList<QString>* getTags();
        CustomColumnLineEdit* getCustomColumnInputAt(int nIndex);
        int getCustomColumnCount();
        QStringList GetTagsList();

    public slots:
        void SelectPoster();
        void addTag();
        void checkValid();
        void CheckTag(QString sTag);

        void clickedTag(Tag* tag);
        void mouseEnteredTag(Tag* tag);
        void mouseLeftTag(Tag* tag);


};

#endif // EDITMOVIEDIALOG_HPP
