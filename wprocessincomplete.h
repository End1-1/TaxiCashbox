#ifndef WPROCESSINCOMPLETE_H
#define WPROCESSINCOMPLETE_H

#include "widget.h"

namespace Ui {
class wProcessIncomplete;
}

class wProcessIncomplete : public Widget
{
    Q_OBJECT

public:
    explicit wProcessIncomplete(QWidget *parent = nullptr);
    ~wProcessIncomplete();

private slots:
    void on_btnNext_clicked();

    void on_btnBalance_clicked();

    void on_btnDept_clicked();

    void on_btnWaybill_clicked();

private:
    Ui::wProcessIncomplete *ui;
};

#endif // WPROCESSINCOMPLETE_H
