QT       += core gui network axcontainer

RC_FILE = res.rc
ICON = app.ico

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    QRCodeGenerator.cpp \
    cashbox.cpp \
    httprequest.cpp \
    lineedit.cpp \
    main.cpp \
    dlg.cpp \
    rkeyboard.cpp \
    wbalancepage.cpp \
    wfinish.cpp \
    widget.cpp \
    wloading.cpp \
    wmoneyinput.cpp \
    wprocessincomplete.cpp \
    wusernamepassword.cpp

HEADERS += \
    QRCodeGenerator.h \
    cashbox.h \
    config.h \
    dlg.h \
    httprequest.h \
    lineedit.h \
    rkeyboard.h \
    wbalancepage.h \
    wfinish.h \
    widget.h \
    wloading.h \
    wmoneyinput.h \
    wprocessincomplete.h \
    wusernamepassword.h

FORMS += \
    dlg.ui \
    rkeyboard.ui \
    wbalancepage.ui \
    wfinish.ui \
    wloading.ui \
    wmoneyinput.ui \
    wprocessincomplete.ui \
    wusernamepassword.ui

TRANSLATIONS += \
    CashBox_ru_RU.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

DISTFILES += \
    res.rc
