#ifndef WBALANCEPAGE_H
#define WBALANCEPAGE_H

#include "widget.h"

namespace Ui {
class wBalancePage;
}

class wBalancePage : public Widget
{
    Q_OBJECT

public:
    explicit wBalancePage(QWidget *parent = nullptr);
    ~wBalancePage();

private slots:
    void nextWaybillPage(bool error, const QString &d);

    void on_btnNext_clicked();

    void on_btnFillBalance_clicked();

    void on_btnDept_clicked();

    void on_btnWaybill_clicked();

    void on_btnExit_clicked();

    void on_btnUP_clicked();

    void on_btnDown_clicked();

private:
    Ui::wBalancePage *ui;
};

#endif // WBALANCEPAGE_H
