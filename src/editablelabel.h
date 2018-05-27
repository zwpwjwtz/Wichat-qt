#ifndef EDITABLELABEL_H
#define EDITABLELABEL_H

#include <QWidget>

namespace Ui {
class EditableLabel;
}

class EditableLabel : public QWidget
{
    Q_OBJECT

public:
    explicit EditableLabel(QWidget *parent = 0);
    ~EditableLabel();

    QString text();
    void setText(QString text);
    void setTextEditable(bool editable);

signals:
    void textChanged(QString text);

protected:
    void resizeEvent(QResizeEvent* event);

private slots:
    void on_pushButton_clicked();

private:
    Ui::EditableLabel *ui;
    bool textEditable;
    bool editing;
};

#endif // EDITABLELABEL_H
