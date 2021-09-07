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
    if (jo["debt"].toDouble() > 0.01) {
        ui->btnWaybill->setEnabled(false);
    }
    ui->lbMessage->setVisible(false);
    ui->lbTextWarning->setVisible(jo["debt"].toDouble() > 0.01);
}

wBalancePage::~wBalancePage()
{
    delete ui;
}

void wBalancePage::on_btnNext_clicked()
{

}

void wBalancePage::on_btnFillBalance_clicked()
{
    fDlg->getMoney(1);
}

void wBalancePage::on_btnDept_clicked()
{
    fDlg->getMoney(2);
}

void wBalancePage::on_btnWaybill_clicked()
{
    fDlg->getMoney(3);
}

void wBalancePage::on_btnExit_clicked()
{
    fDlg->firstPage();
}
