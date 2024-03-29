#include "wmoneyinput.h"
#include "ui_wmoneyinput.h"

wMoneyInput::wMoneyInput(QWidget *parent) :
    Widget(parent),
    ui(new Ui::wMoneyInput)
{
    ui->setupUi(this);
    fTimer = new QTimer(this);
    connect(fTimer, SIGNAL(timeout()), this, SLOT(timeout()));
    fCloseTimeout = 0;
    fTimer->start(2000);
}

wMoneyInput::~wMoneyInput()
{
    delete ui;
}

void wMoneyInput::totalSum(double a, double b, double c)
{
    if (a > 0.0001) {
        fCloseTimeout = 1;
    }

    ui->lbDeposit->setVisible(false);
    ui->lbNeeded->setVisible(b > 0.0001 && (fDlg->fMode == 3));
    ui->lbRemain->setVisible(c > -1 && (fDlg->fMode == 3));

    double dep = 0;
    if (fDlg->fDeposit > 0) {
        if (fDlg->fMode == 2 || fDlg->fMode == 3) {
            ui->lbDeposit->setVisible(true);
            dep = (fDlg->fMode == 2) ? fDlg->FDept - a : fDlg->FNeeded - a;
            if (dep < 0) {
                dep = 0;
            } else {
                dep = dep > fDlg->fDeposit ? fDlg->fDeposit : dep;
            }
            ui->lbDeposit->setText(QString("%1 %2 %3 %4").arg(tr("Deposit used")).arg(dep).arg(tr("From")).arg(fDlg->fDeposit));
        }
    }

    ui->btnBack->setEnabled(a < 0.01);
    ui->lbAmount->setText(QString("%1 %2 %3").arg(tr("Received")).arg(float_str(a, 2)).arg(tr("RUB")));
    ui->lbNeeded->setText(QString("%1 %2 %3").arg(tr("Needed")).arg(float_str(b, 2)).arg(tr("RUB")));
    ui->lbRemain->setText(QString("%1 %2 %3").arg(tr("Remain")).arg(float_str(c - dep, 2)).arg(tr("RUB")));
    ui->btnNext->setEnabled(c - dep < 0.001);
    ui->btnDeposit->setEnabled(true);
    ui->btnDeposit->setEnabled(a > 0.001);
    ui->btn1000->setEnabled(true);
    ui->btn100->setEnabled(true);
}

void wMoneyInput::timeout()
{
    if (fCloseTimeout == 0) {
        fDlg->startReceiveMoney();
    }
    fCloseTimeout++;
    if (fCloseTimeout > 14) {
        ui->btnNext->click();
    }
}

void wMoneyInput::on_btnNext_clicked()
{
    ui->btnBack->setEnabled(false);
    ui->btn100->setEnabled(false);
    ui->btn1000->setEnabled(false);
    ui->btnDeposit->setEnabled(false);
    ui->btnNext->setEnabled(false);
    fDlg->stopReceiveMoney();
    //qApp->processEvents();
    fTimer->stop();
    fDlg->makePayment();
}

void wMoneyInput::on_btn100_clicked()
{
    ui->btnDeposit->setEnabled(false);
    ui->btn100->setEnabled(false);
    ui->btn1000->setEnabled(false);
    fDlg->bill(100, true);
}

void wMoneyInput::on_btn1000_clicked()
{
    ui->btnDeposit->setEnabled(false);
    ui->btn1000->setEnabled(false);
    ui->btn100->setEnabled(false);
    fDlg->bill(1000, true);
}

void wMoneyInput::on_btnDeposit_clicked()
{

    ui->btnNext->setEnabled(false);
    ui->btnDeposit->setEnabled(false);
    fDlg->stopReceiveMoney();
    //qApp->processEvents();
    fTimer->stop();
    fDlg->fMode = 1;
    fDlg->makePayment();
}

void wMoneyInput::on_btnBack_clicked()
{
    //fDlg->firstPage();
    fDlg->stopReceiveMoney();
    //qApp->processEvents();
    //fTimer->stop();
    ui->btnDeposit->setEnabled(false);
    ui->btn100->setEnabled(false);
    ui->btn1000->setEnabled(false);
    fDlg->userPage();
}
