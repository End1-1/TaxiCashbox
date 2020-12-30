#include "widget.h"
#include "dlg.h"
#include <QSettings>

static QSettings settings("YELLOWTAX", "TERMINAL");

Widget::Widget(QWidget *parent) : QWidget(parent)
{
    fDlg = static_cast<Dlg*>(parent);
}

QVariant Widget::_s(const QString &name) const
{
    return settings.value(name);
}
