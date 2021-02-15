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
    void on_btnNext_clicked();

    void on_btnFillBalance_clicked();

    void on_btnDept_clicked();

    void on_btnWaybill_clicked();

    void on_btnExit_clicked();

private:
    Ui::wBalancePage *ui;
};

#endif // WBALANCEPAGE_H
