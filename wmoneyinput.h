#ifndef WMONEYINPUT_H
#define WMONEYINPUT_H

#include "widget.h"
#include <QTimer>

namespace Ui {
class wMoneyInput;
}

class wMoneyInput : public Widget
{
    Q_OBJECT

public:
    explicit wMoneyInput(QWidget *parent = nullptr);
    ~wMoneyInput();

public slots:
    void totalSum(double a, double b, double c);

private slots:
    void timeout();
    void on_btnNext_clicked();
    void on_btn100_clicked();
    void on_btn1000_clicked();

private:
    Ui::wMoneyInput *ui;
    QTimer *fTimer;
    int fCloseTimeout;
};

#endif // WMONEYINPUT_H
