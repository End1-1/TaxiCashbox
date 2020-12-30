#include "wloading.h"
#include "ui_wloading.h"
#include <QMovie>

wLoading::wLoading(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::wLoading)
{
    ui->setupUi(this);
    QMovie *m = new QMovie(":/res/loading.gif");
    ui->label->setMovie(m);
    m->start();
}

wLoading::~wLoading()
{
    delete ui;
}
