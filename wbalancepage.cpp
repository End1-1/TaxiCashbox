#include "wbalancepage.h"
#include "ui_wbalancepage.h"
#include "dlg.h"
#include "config.h"
#include "httprequest.h"
#include <QMessageBox>
#include <QJsonObject>
#include <QSettings>
#include <QJsonDocument>

wBalancePage::wBalancePage(QWidget *parent) :
    Widget(parent),
    ui(new Ui::wBalancePage)
{
    ui->setupUi(this);
    QJsonDocument jdoc = QJsonDocument::fromJson(_s("balance").toString().toUtf8());
    QJsonObject jo = jdoc.object();
    ui->leBalance->setText(float_str(jo["balance"].toDouble(), 2));
    ui->leDebt->setText(float_str(jo["debt"].toDouble(), 2));
    if (ui->leDebt->text() == "0") {
        ui->btnDept->setEnabled(false);
    }
    if (jo["debt"].toDouble() > 0.01 || fDlg->fWaybillMax < 1) {
        ui->btnWaybill->setEnabled(false);
        ui->wspin->setEnabled(false);
    }
    ui->lbMessage->setVisible(false);
    ui->lbTextWarning->setVisible(jo["debt"].toDouble() > 0.01);
}

wBalancePage::~wBalancePage()
{
    delete ui;
}

void wBalancePage::nextWaybillPage(bool error, const QString &d)
{
    if (error) {
        QMessageBox::critical(this, tr("Error"), d);
        return;
    }
    QJsonObject o = QJsonDocument::fromJson(d.toUtf8()).object();
    fDlg->FNeeded = o["waybills_price"].toDouble();
    fDlg->getMoney(3);
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
    fDlg->fWaybillSelected = ui->leWaybills->text().toInt();
    HttpRequest *hr = new HttpRequest(QString("https://%1/app/terminal/selected_waybills_price?days=%2")
                                      .arg(server)
                                      .arg(fDlg->fWaybillSelected), SLOT(nextWaybillPage(bool, QString)), this);
    hr->setHeader("Authorization", "Bearer " + _s("driver_token").toString());
    hr->getRequest();
}

void wBalancePage::on_btnExit_clicked()
{
    fDlg->firstPage();
}

void wBalancePage::on_btnUP_clicked()
{
    ui->leWaybills->setText(QString::number(ui->leWaybills->text().toInt() + 1));
    if (ui->leWaybills->text().toInt() > fDlg->fWaybillMax) {
        ui->leWaybills->setText(QString::number(fDlg->fWaybillMax));
    }
    fDlg->fWaybillSelected = ui->leWaybills->text().toInt();
}

void wBalancePage::on_btnDown_clicked()
{
    ui->leWaybills->setText(QString::number(ui->leWaybills->text().toInt() - 1));
    if (ui->leWaybills->text().toInt() < 1) {
        ui->leWaybills->setText("1");
    }
    fDlg->fWaybillSelected = ui->leWaybills->text().toInt();
}

