#ifndef WLOADING_H
#define WLOADING_H

#include <QWidget>

namespace Ui {
class wLoading;
}

class wLoading : public QWidget
{
    Q_OBJECT

public:
    explicit wLoading(QWidget *parent = nullptr);
    ~wLoading();

private:
    Ui::wLoading *ui;
};

#endif // WLOADING_H
