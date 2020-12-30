#ifndef WFINISH_H
#define WFINISH_H

#include "widget.h"
#include <QTimer>

namespace Ui {
class wFinish;
}

class wFinish : public Widget
{
    Q_OBJECT

public:
    explicit wFinish(QWidget *parent = nullptr);
    ~wFinish();
    void setMessage(const QString &msg);

private slots:
    void timeout();

private:
    Ui::wFinish *ui;
    QTimer fTimer;
};

#endif // WFINISH_H
