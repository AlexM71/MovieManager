#ifndef TAG_HPP
#define TAG_HPP

#include <QLabel>

class Tag : public QLabel {

    Q_OBJECT

    private:
        QString m_savedTag;
        int m_nWidth;

    public:
        Tag(QString label);
        Tag(Tag* tag);

        int getWidth();
        QString getSavedTag();
        void setSavedTag(QString tag);

        void enterEvent(QEnterEvent *event);
        void leaveEvent(QEvent *event);

    signals:
        void clicked(Tag* tag);
        void mouseEnter(Tag* tag);
        void mouseLeave(Tag* tag);

    public slots:
        void mousePressEvent(QMouseEvent* event);

};

#endif // TAG_HPP
