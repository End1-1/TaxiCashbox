#include "wprocessincomplete.h"
#include "ui_wprocessincomplete.h"

wProcessIncomplete::wProcessIncomplete(QWidget *parent) :
    Widget(parent),
    ui(new Ui::wProcessIncomplete)
{
    ui->setupUi(this);
    ui->lbAmount->setText(float_str(fDlg->fTransactionAmount, 2));
}

wProcessIncomplete::~wProcessIncomplete()
{
    delete ui;
}

void wProcessIncomplete::on_btnNext_clicked()
{
    if (!ui->btnBalance->isChecked() && !ui->btnDept->isChecked() && !ui->btnWaybill->isChecked()) {
        return;
    }
    int mode = 1;
    if (ui->btnBalance->isChecked()) {
        mode = 1;
    } else if (ui->btnDept->isChecked()) {
        mode = 2;
    } else if (ui->btnWaybill->isChecked()) {
        mode = 3;
    }
    fDlg->makePayment(mode, fDlg->fTransactionAmount);
}

void wProcessIncomplete::on_btnBalance_clicked()
{
    ui->btnDept->setChecked(false);
    ui->btnWaybill->setChecked(false);
}

void wProcessIncomplete::on_btnDept_clicked()
{
    ui->btnBalance->setChecked(false);
    ui->btnWaybill->setChecked(false);
}

void wProcessIncomplete::on_btnWaybill_clicked()
{
    ui->btnBalance->setChecked(false);
    ui->btnDept->setChecked(false);
}
