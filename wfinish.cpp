#include "wfinish.h"
#include "ui_wfinish.h"

wFinish::wFinish(QWidget *parent) :
    Widget(parent),
    ui(new Ui::wFinish)
{
    ui->setupUi(this);
    connect(&fTimer, SIGNAL(timeout()), this, SLOT(timeout()));
    fTimer.start(5000);
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
    fDlg->firstPage();
}
