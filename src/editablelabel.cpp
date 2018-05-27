#include "editablelabel.h"
#include "ui_editablelabel.h"

EditableLabel::EditableLabel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EditableLabel)
{
    ui->setupUi(this);
    ui->text->hide();
    editing = false;
}

EditableLabel::~EditableLabel()
{
    delete ui;
}

QString EditableLabel::text()
{
    return ui->label->text();
}

void EditableLabel::setText(QString text)
{
    ui->label->setText(text);
    ui->text->setText(text);
}

void EditableLabel::setTextEditable(bool editable)
{
    textEditable = editable;
    if (editable)
        ui->pushButton->show();
    else
        ui->pushButton->hide();
}

void EditableLabel::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    ui->text->resize(ui->frameEdit->width() - 2,
                     ui->frameEdit->height());
    ui->label->resize(ui->frameEdit->width() - 2,
                      ui->frameEdit->height());

    int buttonHeight = ui->pushButton->height();
    if (buttonHeight > ui->pushButton->width())
        buttonHeight = ui->pushButton->width();
    ui->pushButton->setIconSize(QSize(buttonHeight * 0.9,
                                      buttonHeight * 0.9));
}

void EditableLabel::on_pushButton_clicked()
{
    if (!textEditable)
        return;
    if (editing)
    {
        ui->text->hide();
        if (ui->label->text() != ui->text->text())
        {
            emit textChanged(ui->text->text());
            ui->label->setText(ui->text->text());
        }
        ui->label->show();
        ui->pushButton->setIcon(QIcon(":/Icons/edit.png"));
        editing = false;
    }
    else
    {
        ui->text->show();
        ui->text->setFocus();
        ui->label->hide();
        ui->pushButton->setIcon(QIcon(":/Icons/save.png"));
        editing = true;
    }
}
