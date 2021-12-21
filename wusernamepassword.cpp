#include "wusernamepassword.h"
#include "ui_wusernamepassword.h"
#include "dlg.h"

wUsernamePassword::wUsernamePassword(QWidget *parent) :
    Widget(parent),
    ui(new Ui::wUsernamePassword)
{
    ui->setupUi(this);
    fDlg = static_cast<Dlg*>(parent);
    connect(ui->leUsername, SIGNAL(focusIn()), this, SLOT(lineEditFocus()));
    connect(ui->lePassword, SIGNAL(focusIn()), this, SLOT(lineEditFocus()));
    ui->leUsername->setFocus();
#ifndef QT_DEBUG
    ui->lePassword->setEchoMode(QLineEdit::Password);
#endif
}

wUsernamePassword::~wUsernamePassword()
{
    delete ui;
}

void wUsernamePassword::lineEditFocus()
{
    ui->wkbd->setLineEdit(static_cast<LineEdit*>(sender()));
}

void wUsernamePassword::on_btnNext_clicked()
{
    fDlg->requestAuthDriver(ui->leUsername->text(), ui->lePassword->text());
}
