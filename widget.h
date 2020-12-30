#ifndef WIDGET_H
#define WIDGET_H

#include "dlg.h"
#include <QWidget>
#include <QLocale>
#include <QDebug>

#define float_str(value, f) QLocale().toString(value, 'f', f).remove(QRegExp("(?!\\d[\\.\\,][1-9]+)0+$")).remove(QRegExp("[\\.\\,]$"))

class Widget : public QWidget
{
    Q_OBJECT
public:
    explicit Widget(QWidget *parent = nullptr);
    QVariant _s(const QString &name) const;

protected:
    Dlg *fDlg;

signals:

};

#endif // WIDGET_H
