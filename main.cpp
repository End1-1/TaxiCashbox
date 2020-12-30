#include "dlg.h"
#include <QTranslator>
#include <QStyleFactory>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator t;
    t.load(":/CashBox.qm");
    a.installTranslator(&t);
    a.setStyle(QStyleFactory::create("fusion"));


    Dlg w;
#ifdef QT_DEBUG
    w.showFullScreen();
#else
    w.showFullScreen();
#endif
    return a.exec();
}
