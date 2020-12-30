#ifndef WUSERNAMEPASSWORD_H
#define WUSERNAMEPASSWORD_H

#include "widget.h"

namespace Ui {
class wUsernamePassword;
}

class wUsernamePassword : public Widget
{
    Q_OBJECT

public:
    explicit wUsernamePassword(QWidget *parent = nullptr);
    ~wUsernamePassword();

private slots:
    void lineEditFocus();
    void on_btnNext_clicked();

private:
    Ui::wUsernamePassword *ui;

};

#endif // WUSERNAMEPASSWORD_H
