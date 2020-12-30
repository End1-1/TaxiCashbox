#ifndef DLG_H
#define DLG_H

#include <QDialog>
#include <windows.h>

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

    void printWaybill(QJsonObject o);
    void requestAuthDriver(const QString &username, const QString &password);
    void startReceiveMoney();
    void stopReceiveMoney();
    void getMoney(int mode);
    void addCash(double cash);
    void makePayment();
    void firstPage();
    void makePayment(int mode, double cash);
    double fTransactionAmount;
    virtual void closeEvent(QCloseEvent *e) override;

public slots:
    void bill(WORD sum, bool canLoop);

private slots:
    void auth(bool error, const QString &data);
    void authDriver(bool error, const QString &data);
    void addCashResponse(bool error, const QString &data);
    void payResponse(bool error, const QString &data);
    void debts(bool error, const QString &data);
    void on_pushButton_clicked();
    void message(int code, const QString &msg);

private:
    Ui::Dlg *ui;
    QWidget *addWidget(QWidget *w);
    void removeWidget();

signals:
    void totalSum(double a, double b, double c);

};
#endif // DLG_H
