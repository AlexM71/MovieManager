#ifndef LOG_HPP
#define LOG_HPP

#include <QList>
#include <QObject>
#include <QDate>
#include <QTime>
#include <Structures.hpp>


class Log : public QObject {

    Q_OBJECT

    private:
       QList<stLogElement> m_log;

    public:
        Log();
        void append(QString s, enum eLog criticity);
        void clear();
        stLogElement getLog(int i);
        int size();

    signals:
        void logAppended();


};

#endif // LOG_HPP
