#ifndef DLG_H
#define DLG_H

#include "cashbox.h"
#include <QDialog>
#include <windows.h>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class Dlg; }
QT_END_NAMESPACE


class Dlg : public QDialog
{
    Q_OBJECT

public:
    Dlg(QWidget *parent = nullptr);
    ~Dlg();

    int fMode;
    double FSum;
    double FRemain;
    double FNeeded;
    double FDept;
    double fDeposit;
    int fWaybillMax;
    int fWaybillSelected;

    void printWaybill(QJsonArray ja);
    void requestAuthDriver(const QString &username, const QString &password);
    void startReceiveMoney();
    void stopReceiveMoney();
    void getMoney(int mode);
    void addCash(double cash);
    void makePayment();
    void firstPage();
    void userPage();
    void makePayment(int mode, double cash);
    double fTransactionAmount;
    virtual void closeEvent(QCloseEvent *e) override;

public slots:
    void bill(WORD sum, bool canLoop);

private slots:
    void c_endPolling();
    void timeout();
    void auth(bool error, const QString &data);
    void authDriver(bool error, const QString &data);
    void addCashResponse(bool error, const QString &data);
    void payResponse(bool error, const QString &data);
    void debts(bool error, const QString &data);
    void on_pushButton_clicked();
    void message(int code, const QString &msg);
    void deleteFile(bool error, const QString &data);

private:
    Ui::Dlg *ui;
    int fUserPageTimeout;
    QTimer fTimer;
    QWidget *fCurrentWidget;
    QWidget *addWidget(QWidget *w);
    void resetData();

signals:
    void totalSum(double a, double b, double c);
    void c_stopReceiveMoney();
    void c_reset();
    void c_canPollingLoop(bool v);
    void c_enableBillTypes(Nominal n);
    void c_poll();
    void c_pollingLoop(WORD,DWORD);

};
#endif // DLG_H
