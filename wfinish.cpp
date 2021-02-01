#include "wfinish.h"
#include "ui_wfinish.h"

wFinish::wFinish(bool error, QWidget *parent) :
    Widget(parent),
    ui(new Ui::wFinish)
{
    ui->setupUi(this);
    connect(&fTimer, SIGNAL(timeout()), this, SLOT(timeout()));
    fTimer.start(2000);
    fError = error;
}

wFinish::~wFinish()
{
    delete ui;
}

void wFinish::setMessage(const QString &msg)
{
    ui->label->setText(msg);
}

void wFinish::timeout()
{
    if (fError) {
        fDlg->firstPage();
    } else {
        fDlg->userPage();
    }
}
