#include "Log.hpp"

Log::Log() {

}

void Log::append(QString s, enum eLog criticity) {
    QString datetime = QString::number(QDate::currentDate().year()) + "-" + QString::number(QDate::currentDate().month()) + "-" + QString::number(QDate::currentDate().day()) + " " + QString::number(QTime::currentTime().hour()) + ":" + QString::number(QTime::currentTime().minute()) + ":" + QString::number(QTime::currentTime().second());
    struct stLogElement elem;
    elem.sText = tr("%1: %2").arg(datetime, s);
    elem.eCriticity = criticity;
    m_log.append(elem);
    emit logAppended();
}

void Log::clear() {
    m_log.clear();
}

stLogElement Log::getLog(int i) {
    QList<stLogElement>::iterator it = m_log.begin();
    std::advance(it, i);
    return *it;
}

int Log::size() {
    return m_log.size();
}
