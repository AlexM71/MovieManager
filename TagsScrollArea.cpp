#include "TagsScrollArea.hpp"

TagsScrollArea::TagsScrollArea(QWidget* parent) {
    QWidget* TagsWidget = new QWidget();
    QHBoxLayout* TagsLayout = new QHBoxLayout(TagsWidget);
    QSpacerItem* horizontalSpacer = new QSpacerItem(40, 25, QSizePolicy::Expanding, QSizePolicy::Minimum);

    this->setParent(parent);
    this->setObjectName("TagsScrollArea");
    this->setMaximumSize(QSize(16777215, 45));
    this->setFrameShape(QFrame::NoFrame);
    this->setLineWidth(0);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);
    this->setWidgetResizable(true);
    this->setWidget(TagsWidget);

    TagsWidget->setObjectName("TagsWidget");
    TagsWidget->setGeometry(QRect(0, 0, 324, 45));
    TagsWidget->setMaximumSize(QSize(16777215, 45));
    TagsLayout->setObjectName("TagsLayout");
    TagsLayout->setContentsMargins(0, 0, 0, 0);
    TagsLayout->addItem(horizontalSpacer);

}
