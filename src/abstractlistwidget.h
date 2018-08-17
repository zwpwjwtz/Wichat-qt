#ifndef ABSTRACTLISTWIDGET_H
#define ABSTRACTLISTWIDGET_H

#include <QWidget>
#include <QStandardItemModel>


class QMenu;

class AbstractListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AbstractListWidget(QWidget *parent = 0);

    virtual bool refreshList() = 0;
    virtual bool updateList() = 0;
    virtual bool updateEntryInfo() = 0;
    virtual bool search(QString text) = 0;

    virtual void setSearchMode(bool isSearching);
    virtual bool hasMember(QString ID);

signals:
    void listUpdated();
    void entryClicked(QString ID);

protected:
    QMenu* contextMenu;
    QStandardItemModel listModel;
    bool searchMode;
};

#endif // ABSTRACTLISTWIDGET_H
