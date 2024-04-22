#include "Tag.hpp"

Tag::Tag(QString label)
{
    this->setText(label);
    this->setAlignment(Qt::AlignHCenter);

    QFontMetrics fm(this->fontMetrics());
    m_nWidth = 35;
    if((fm.boundingRect(label).width() + 20) > 35)
        m_nWidth = fm.boundingRect(label).width() + 20;

    this->setFixedWidth(m_nWidth);
    this->setFixedHeight(25);
}
Tag::Tag(Tag* tag) {
    this->setText(tag->text());
    this->setAlignment(Qt::AlignHCenter);
    QFontMetrics fm(this->fontMetrics());
    m_nWidth = 35;
    if((fm.boundingRect(tag->text()).width() + 20) > 35)
        m_nWidth = fm.boundingRect(tag->text()).width() + 20;

    this->setFixedWidth(m_nWidth);
    this->setFixedHeight(25);
}
QString Tag::getSavedTag() {
    return m_savedTag;
}

void Tag::setSavedTag(QString tag) {
    m_savedTag = tag;
}

int Tag::getWidth() {
    return m_nWidth;
}

void Tag::mousePressEvent(QMouseEvent* event) {
    emit clicked(this);
}

void Tag::enterEvent(QEnterEvent *event) {
    emit mouseEnter(this);
}

void Tag::leaveEvent(QEvent *event) {
    emit mouseLeave(this);
}
