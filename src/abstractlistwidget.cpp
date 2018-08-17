#include "abstractlistwidget.h"


AbstractListWidget::AbstractListWidget(QWidget *parent)
    : QWidget(parent)
{
    contextMenu = nullptr;
    searchMode = false;
}

void AbstractListWidget::setSearchMode(bool isSearching)
{
    searchMode = isSearching;
}

bool AbstractListWidget::hasMember(QString ID)
{
    Q_UNUSED(ID)
    return false;
}
