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

    ui->lbAmount->setText(QString("%1 %2 %3").arg(tr("Received")).arg(float_str(a, 2)).arg(tr("RUB")));
    ui->lbNeeded->setText(QString("%1 %2 %3").arg(tr("Needed")).arg(float_str(b, 2)).arg(tr("RUB")));
    ui->lbRemain->setText(QString("%1 %2 %3").arg(tr("Remain")).arg(float_str(c - dep, 2)).arg(tr("RUB")));
    qApp->processEvents();
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
    ui->btnNext->setEnabled(false);
    fDlg->stopReceiveMoney();
    qApp->processEvents();
    fTimer->stop();
    fDlg->makePayment();
}

void wMoneyInput::on_btn100_clicked()
{
    fDlg->bill(100, true);
}

void wMoneyInput::on_btn1000_clicked()
{
    fDlg->bill(1000, true);
}
