#include "wbalancepage.h"
#include "ui_wbalancepage.h"
#include "dlg.h"
#include <QJsonObject>

wBalancePage::wBalancePage(QWidget *parent) :
    Widget(parent),
    ui(new Ui::wBalancePage)
{
    ui->setupUi(this);
    QJsonObject jo = _s("balance").toJsonObject();
    ui->leBalance->setText(float_str(jo["balance"].toDouble(), 2));
    ui->leDebt->setText(float_str(jo["debt"].toDouble(), 2));
    if (ui->leDebt->text() == "0") {
        ui->btnDept->setEnabled(false);
    }
    ui->lbMessage->setVisible(false);
}

wBalancePage::~wBalancePage()
{
    delete ui;
}

void wBalancePage::on_btnNext_clicked()
{
    int mode = 0;
    if (ui->btnFillBalance->isChecked()) {
        mode = 1;
    } else if (ui->btnDept->isChecked()) {
        mode = 2;
    } else if (ui->btnWaybill->isChecked()) {
        mode = 3;
    }
    if (mode == 0) {
        ui->lbMessage->setVisible(true);
        return;
    }
    fDlg->getMoney(mode);
}

void wBalancePage::on_btnFillBalance_clicked()
{
    ui->btnDept->setChecked(false);
    ui->btnWaybill->setChecked(false);
}

void wBalancePage::on_btnDept_clicked()
{
    ui->btnFillBalance->setChecked(false);
    ui->btnWaybill->setChecked(false);
}

void wBalancePage::on_btnWaybill_clicked()
{
    ui->btnFillBalance->setChecked(false);
    ui->btnDept->setChecked(false);
}
