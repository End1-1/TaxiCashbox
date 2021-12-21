#include "dlg.h"
#include "ui_dlg.h"
#include "config.h"
#include "httprequest.h"
#include "wloading.h"
#include "wusernamepassword.h"
#include "wbalancepage.h"
#include "wfinish.h"
#include "wmoneyinput.h"
#include "wprocessincomplete.h"
#include "QRCodeGenerator.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QAxObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QDir>
#include <QMutex>
#include <QFile>
#include <ctime>
#include <QThread>

static CashBox c;
static QSettings _s("YELLOWTAXI", "TERMINAL");
static QMutex fMutex;

Dlg::Dlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dlg)
{
    ui->setupUi(this);

    qRegisterMetaType<Nominal>("Nominal");
    qRegisterMetaType<WORD>("WORD");
    qRegisterMetaType<DWORD>("DWORD");

    connect(&c, SIGNAL(ProcessMessage(int,QString)), this, SLOT(message(int,QString)));
    connect(&c, SIGNAL(PolingBill(WORD,bool)), this, SLOT(bill(WORD,bool)));
    connect(&c, SIGNAL(EndPolling()), this, SLOT(c_endPolling()));
    connect(this, SIGNAL(c_reset()), &c, SLOT(reset()));
    connect(this, SIGNAL(c_canPollingLoop(bool)), &c, SLOT(canPollingLoop(bool)));
    connect(this, SIGNAL(c_enableBillTypes(Nominal)), &c, SLOT(enableBillTypes(Nominal)));
    connect(this, SIGNAL(c_pollingLoop(WORD,DWORD)), &c, SLOT(pollingLoop(WORD,DWORD)));
    connect(this, SIGNAL(c_poll()), &c, SLOT(poll()));
    if (c.openComPort(_s.value("com_port").toString().toUtf8().data())) {
        qDebug() << "OPENED";
    }
    auto *cThread = new QThread();
    c.moveToThread(cThread);
    cThread->start();

    FSum = 0;
    fDeposit = 0;
    fUserPageTimeout = 0;
    fCurrentWidget = nullptr;
    fWaybillSelected = 1;

    addWidget(new wLoading());
    //qApp->processEvents();
    HttpRequest *hr = new HttpRequest(QString("https://%1/app/terminal/auth").arg(server), SLOT(auth(bool, QString)), this);
    hr->fContentType = "application/json";
    QJsonObject jo;
    jo["terminal_name"] = "Terminal1";
    jo["password"] = "nyt_transit_terminal__(@First)../2020";
    hr->fData = QJsonDocument(jo).toJson();
//    hr->setFormData("terminal_name", "Terminal1");
//    hr->setFormData("password", "nyt_transit_terminal__(@First)../2020");
    hr->postRequest();


}

Dlg::~Dlg()
{
    delete ui;
}

void Dlg::printWaybill(QJsonArray ja)
{
    for (int i = 0; i < ja.count(); i++) {
        QJsonObject jo = ja[i].toObject();
    srand(time(0));
    QString r = QString("/ah_%1.xlsm").arg(rand());
    QJsonObject ow = jo["waybills"].toObject();
    QJsonObject oc = jo["car"].toObject();
    QJsonObject od = jo["driver"].toObject();
    QJsonObject op = jo["park"].toObject();
    QDateTime startDateTime = QDateTime::fromString(ow["start_date"].toString(), "yyyy-MM-dd HH:mm");
    QDateTime endDateTime = QDateTime::fromString(ow["end_date"].toString(), "yyyy-MM-dd HH:mm");
    QFile srcFile(qApp->applicationDirPath() + "/wb.xlsm");
    QDir dstDir = QDir::tempPath();
    QFile dstFile(dstDir.absolutePath() + r);
    if (dstFile.exists()) {
        if (!dstFile.remove()) {
            QMessageBox::critical(0, tr("Error"), tr("Remove file error") + "<br>" + dstFile.errorString());
        }
    }
    if (!srcFile.copy(dstFile.fileName())) {
        QMessageBox::critical(0, tr("Error"), tr("File copy error") + "<br>" + srcFile.errorString());
        return;
    }

    int levelIndex = 1;
    int versionIndex = 0;
    bool bExtent = true;
    int maskIndex = -1;
    QString encodeString = QString("%1").arg(ow["number"].toString());
    CQR_Encode qrEncode;
    bool successfulEncoding = qrEncode.EncodeData( levelIndex, versionIndex, bExtent, maskIndex, encodeString.toUtf8().data() );
    if (!successfulEncoding) {
        //fLog.append("Cannot encode qr image");
    }
    int qrImageSize = qrEncode.m_nSymbleSize;
    int encodeImageSize = qrImageSize + ( QR_MARGIN * 2 );
    QImage encodeImage(encodeImageSize, encodeImageSize, QImage::Format_Mono);
    encodeImage.fill(1);

    for ( int i = 0; i < qrImageSize; i++ ) {
        for ( int j = 0; j < qrImageSize; j++ ) {
            if ( qrEncode.m_byModuleData[i][j] ) {
                encodeImage.setPixel(i + QR_MARGIN, j + QR_MARGIN, 0);
            }
        }
    }
    QPixmap pix = QPixmap::fromImage(encodeImage);
    pix = pix.scaled(100, 100);
    pix.save(QDir::tempPath() + "/qr.png", "png", 100);

    //QMessageBox::information(this, "", dstFile.fileName().toUtf8());
    QAxObject* excel = new QAxObject("Excel.Application", this);

    excel->dynamicCall("SetVisible(bool)", FALSE);
    QAxObject *workbooks = excel->querySubObject("Workbooks");
    QAxObject *workbook = workbooks->querySubObject( "Open(const QString&)", dstFile.fileName().toUtf8());
    QAxObject *sheets = workbook->querySubObject("Sheets");
    QAxObject *statSheet = sheets->querySubObject("Item(const QVariant&)", QVariant("sh"));
    statSheet->dynamicCall("Select()");

    QAxObject *range = statSheet->querySubObject( "Range(const QVariant&)", QVariant( QString("BI3:BI3")));
    range->dynamicCall( "SetValue(const QVariant&)", ow["number"].toString());
    range = statSheet->querySubObject( "Range(const QVariant&)", QVariant( QString("BI70:BI70")));
    range->dynamicCall( "SetValue(const QVariant&)", ow["number"].toString());

    range = statSheet->querySubObject("Range(const QVariant&)", QVariant(QString("O7")));
    range->dynamicCall("SetValue(const QVariant&)", QString("%1").arg(op["entity"].toObject()["full_title"].toString()));
    range = statSheet->querySubObject("Range(const QVariant&)", QVariant(QString("O74")));
    range->dynamicCall("SetValue(const QVariant&)", QString("%1 %2").arg(op["entity"].toObject()["full_title"].toString()));

    range = statSheet->querySubObject("Range(const QVariant&)", QVariant(QString("DH13:DH13")));
    range->dynamicCall("SetValue(const QVariant&)", QString("%1").arg(oc["garage_number"].toInt()));
    range = statSheet->querySubObject("Range(const QVariant&)", QVariant(QString("DH80:DH80")));
    range->dynamicCall("SetValue(const QVariant&)", QString("%1").arg(oc["garage_number"].toInt()));

    range = statSheet->querySubObject("Range(const QVariant&)", QVariant(QString("T17")));
    range->dynamicCall("SetValue(const QVariant&)", QString("%1").arg(od["license_code"].toString()));
    range = statSheet->querySubObject("Range(const QVariant&)", QVariant(QString("T84")));
    range->dynamicCall("SetValue(const QVariant&)", QString("%1").arg(od["license_code"].toString()));

    range = statSheet->querySubObject("Range(const QVariant&)", QVariant(QString("DY66")));
    range->dynamicCall("SetValue(const QVariant&)", QString("%1 %2").arg(tr("Taked")).arg(QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss")));
    range = statSheet->querySubObject("Range(const QVariant&)", QVariant(QString("DY133")));
    range->dynamicCall("SetValue(const QVariant&)", QString("%1 %2").arg(tr("Taked")).arg(startDateTime.addSecs(12 * 60 * 60).toString("HH:mm")));

    range = statSheet->querySubObject( "Range(const QVariant&)", QVariant( QString("C6:C6")));
    range->dynamicCall( "SetValue(const QVariant&)", QString("%1 %2 %3 %4")
                        .arg(tr("From"))
                        .arg(startDateTime.toString("dd/MM/yyyy"))
                        .arg(tr("To"))
                        .arg(startDateTime.addSecs(12 * 60 * 60).toString("dd/MM/yyyy")));

    range = statSheet->querySubObject( "Range(const QVariant&)", QVariant( QString("C73:C73")));
    range->dynamicCall( "SetValue(const QVariant&)", QString("%1 %2 %3 %4")
                        .arg(tr("From"))
                        .arg(startDateTime.addSecs(12 * 60 * 60).toString("dd/MM/yyyy"))
                        .arg(tr("To"))
                        .arg(endDateTime.toString("dd/MM/yyyy")));

    range = statSheet->querySubObject( "Range(const QVariant&)", QVariant( QString("R11:R11")));
    range->dynamicCall( "SetValue(const QVariant&)", oc["vin"].toString());
    range = statSheet->querySubObject( "Range(const QVariant&)", QVariant( QString("R78:R78")));
    range->dynamicCall( "SetValue(const QVariant&)", oc["vin"].toString());

//    range = statSheet->querySubObject( "Range(const QVariant&)", QVariant( QString("AD13:AD13")));
//    range->dynamicCall( "SetValue(const QVariant&)", oc["vin"].toString());
//    range = statSheet->querySubObject( "Range(const QVariant&)", QVariant( QString("AD80:AD80")));
//    range->dynamicCall( "SetValue(const QVariant&)", oc["vin"].toString());

    range = statSheet->querySubObject( "Range(const QVariant&)", QVariant( QString("L14:L14")));
    range->dynamicCall( "SetValue(const QVariant&)", od["surname"].toString() + " " + od["name"].toString() + " " + od["patronymic"].toString());
    range = statSheet->querySubObject( "Range(const QVariant&)", QVariant( QString("L81:L81")));
    range->dynamicCall( "SetValue(const QVariant&)", od["surname"].toString() + " " + od["name"].toString() + " " + od["patronymic"].toString());

    range = statSheet->querySubObject( "Range(const QVariant&)", QVariant( QString("T10:T10")));
    range->dynamicCall( "SetValue(const QVariant&)", oc["mark"].toString() + " " + oc["model"].toString());
    range = statSheet->querySubObject( "Range(const QVariant&)", QVariant( QString("T77:T77")));
    range->dynamicCall( "SetValue(const QVariant&)", oc["mark"].toString() + " " + oc["model"].toString());

    range = statSheet->querySubObject( "Range(const QVariant&)", QVariant( QString("DH11:DH11")));
    range->dynamicCall( "SetValue(const QVariant&)", od["kis_art"].toString());
    range = statSheet->querySubObject( "Range(const QVariant&)", QVariant( QString("DH78:DH78")));
    range->dynamicCall( "SetValue(const QVariant&)", od["kis_art"].toString());

    range = statSheet->querySubObject( "Range(const QVariant&)", QVariant( QString("T17:T17")));
    range->dynamicCall( "SetValue(const QVariant&)", oc["udo_num"].toInt());
    range = statSheet->querySubObject( "Range(const QVariant&)", QVariant( QString("T84:T84")));
    range->dynamicCall( "SetValue(const QVariant&)", oc["udo_num"].toInt());

    range = statSheet->querySubObject( "Range(const QVariant&)", QVariant( QString("V35:T35")));
    range->dynamicCall( "SetValue(const QVariant&)", oc["speedometer"].toInt());
    range = statSheet->querySubObject( "Range(const QVariant&)", QVariant( QString("V102:V102")));
    range->dynamicCall( "SetValue(const QVariant&)", oc["speedometer"].toInt());

    range = statSheet->querySubObject("Range(const QVariant&)", QVariant( QString("AD13:AD13")));
    range->dynamicCall( "SetValue(const QVariant&)", oc["state_license_plate"].toString());
    range = statSheet->querySubObject("Range(const QVariant&)", QVariant( QString("AD80:AD80")));
    range->dynamicCall( "SetValue(const QVariant&)", oc["state_license_plate"].toString());

    range = statSheet->querySubObject("Range(const QVariant&)", QVariant( QString("B22:B22")));
    range->dynamicCall( "SetValue(const QVariant&)", startDateTime.toString("HH:mm"));
    range = statSheet->querySubObject("Range(const QVariant&)", QVariant( QString("B89:B89")));
    range->dynamicCall( "SetValue(const QVariant&)", startDateTime.addSecs(12 * 60 * 60).toString("HH:mm"));

    range = statSheet->querySubObject("Range(const QVariant&)", QVariant( QString("Z22:Z22")));
    range->dynamicCall( "SetValue(const QVariant&)", startDateTime.addSecs(12 * 60 * 60).toString("HH:mm"));
    range = statSheet->querySubObject("Range(const QVariant&)", QVariant( QString("Z89:Z89")));
    range->dynamicCall( "SetValue(const QVariant&)", endDateTime.toString("HH:mm"));


    range = statSheet->querySubObject("Range(const QVariant&)", QVariant( QString("DG28:DG28")));
    range->dynamicCall( "SetValue(const QVariant&)", od["surname"].toString() + " " + od["name"].toString().left(1) + ". " + od["patronymic"].toString().left(1) + ".");
    range = statSheet->querySubObject("Range(const QVariant&)", QVariant( QString("DG95:DG95")));
    range->dynamicCall( "SetValue(const QVariant&)", od["surname"].toString() + " " + od["name"].toString().left(1) + ". " + od["patronymic"].toString().left(1) + ".");

    range = statSheet->querySubObject("Range(const QVariant&)", QVariant( QString("HK57:HK57")));
    range->dynamicCall( "SetValue(const QVariant&)", od["surname"].toString() + " " + od["name"].toString().left(1) + ". " + od["patronymic"].toString().left(1) + ".");
    range = statSheet->querySubObject("Range(const QVariant&)", QVariant( QString("HK124:HK124")));
    range->dynamicCall( "SetValue(const QVariant&)", od["surname"].toString() + " " + od["name"].toString().left(1) + ". " + od["patronymic"].toString().left(1) + ".");
    statSheet->dynamicCall("PrintOut(int,int,int,bool,String)", 1, 100, 1, QVariant(false), "Canon");

    workbook->dynamicCall("Save()", false);
    workbook->dynamicCall("Close(QVariant)", false);
    excel->dynamicCall("Quit()");

    HttpRequest *hr = new HttpRequest(QString("https://%1/app/terminal/upload_waybill").arg(server),
                                      SLOT(deleteFile(bool,QString)), this);
    hr->setHeader("Authorization", "Bearer " + _s.value("driver_token").toString());
    hr->setProperty("file", dstFile.fileName());
    hr->setProperty("qr", QDir::tempPath() + "/qr.png");
    hr->setFormData("transaction_id", QString::number(jo["transaction_id"].toInt()));
    hr->setFileName("waybill",  dstFile.fileName());
    hr->postRequest();

    delete range;
    delete statSheet;
    delete sheets;
    delete workbook;
    delete workbooks;
    delete excel;
    }
}

void Dlg::requestAuthDriver(const QString &username, const QString &password)
{
    addWidget(new wLoading());
    //qApp->processEvents();
    HttpRequest *hr = new HttpRequest(QString("https://%1/app/terminal/auth_driver").arg(server),
                                      SLOT(authDriver(bool, QString)), this);
    hr->setHeader("Authorization", "Bearer " + _s.value("token").toString());
    hr->setFormData("login", username);
    hr->setFormData("password", password);
    hr->postRequest();
}

void Dlg::startReceiveMoney()
{
    WORD FSum = 0;
    Nominal n;
    
    //  CashCodeBillValidatorCCNET.Reset;
    // Óñòàíîâèì ïðèíèìàåìûå êóïþðû
    n.B10 = true;
    n.B50 = true;
    n.B100 = true;
    n.B500 = true;
    n.B1000 = true;
    n.B5000 = true;
    emit c_enableBillTypes(n);

    emit c_pollingLoop((WORD)150000, 50);
    qDebug() << QString("%1").arg(FSum);
}

void Dlg::stopReceiveMoney()
{
    emit c_canPollingLoop(false);
}

void Dlg::getMoney(int mode)
{
    fMode = mode;
    FRemain = FNeeded;
    wMoneyInput *w = new wMoneyInput(this);
    connect(this, SIGNAL(totalSum(double,double,double)), w, SLOT(totalSum(double,double,double)));
    switch (mode) {
    case 1:
        w->totalSum(0, -1, -1);
        break;
    case 2:
        w->totalSum(0, -1, -1);
        break;
    case 3:
        w->totalSum(0, FNeeded, FRemain);
        break;
    }

    addWidget(w);
}

void Dlg::auth(bool error, const QString &data)
{
    if (error) {
        qDebug() << data;
        QMessageBox::critical(this, tr("Error"), data);
        return;
    }
    QJsonDocument jdoc = QJsonDocument::fromJson(data.toUtf8());
    QJsonObject jo = jdoc.object();
    _s.setValue("token", jo["token"].toString());
    addWidget(new wUsernamePassword(this));
}

void Dlg::authDriver(bool error, const QString &data)
{
    QJsonDocument jdoc = QJsonDocument::fromJson(data.toUtf8());
    QJsonObject jo = jdoc.object();
    if (error) {
        qDebug() << data;
        wFinish *w = new wFinish(true, this);
        w->setMessage(jo["message"].toString());
        addWidget(w);
        return;
    }
    _s.setValue("driver_token", jo["access_token"].toString());
    addWidget(new wLoading());
    HttpRequest *hr = new HttpRequest(QString("https://%1/app/terminal/get_debts").arg(server), SLOT(debts(bool, QString)), this);
    hr->setHeader("Authorization", "Bearer " + _s.value("driver_token").toString());
    hr->getRequest();
}

void Dlg::addCashResponse(bool error, const QString &data)
{
    if (error) {
        return;
    }
    QJsonObject jo = QJsonDocument::fromJson(data.toUtf8()).object();
    startReceiveMoney();
}

void Dlg::payResponse(bool error, const QString &data)
{
    if (error) {
        QMessageBox::critical(this, tr("Error"), data);
        addWidget(new wLoading());
        HttpRequest *hr = new HttpRequest(QString("https://%1/app/terminal/get_debts").arg(server), SLOT(debts(bool, QString)), this);
        hr->setHeader("Authorization", "Bearer " + _s.value("driver_token").toString());
        hr->getRequest();
        return;
    }
    addWidget(new wFinish(false, this));
    if (fMode == 3) {
        printWaybill(QJsonDocument::fromJson(data.toUtf8()).array());
    }
}

void Dlg::debts(bool error, const QString &data)
{
    if (error) {
        return;
    }
    QJsonDocument jdoc = QJsonDocument::fromJson(data.toUtf8());
    QJsonObject jo = jdoc.object();
    fTransactionAmount = jo["transaction_cash"].toDouble();
    FNeeded = jo["minimal_repayment_waybill"].toDouble();
    FDept = jo["debt"].toDouble();
    fDeposit = jo["balance"].toDouble();
    fWaybillMax = jo["waybills_allowed_limit"].toInt();
    if (fTransactionAmount > 0.01) {
        addWidget(new wProcessIncomplete(this));
        return;
    }
    _s.setValue("balance", data);
    addWidget(new wBalancePage(this));
}


void Dlg::on_pushButton_clicked()
{
//    WORD FSum = 0;
//    Nominal n;


//    //  CashCodeBillValidatorCCNET.Reset;

//      // Óñòàíîâèì ïðèíèìàåìûå êóïþðû
//      n.B10   =   true;
//      n.B50  =   true;
//      n.B100  =   true;
//      n.B500 =   true;
//      n.B1000 =   true;
//      n.B5000 =   true;

//      c.EnableBillTypes(n);

//      FSum = c.PollingLoop(150, 50);
//      ui->teLog->setText(QString("%1").arg(FSum));
//      ui->teLog->append("Received: " + QString("%1").arg(FSum));

//      // Çàïðåòèì ïðèåì âñåõ êóïþð
//      n.B10   =   false;
//      n.B50   =   false;
//      n.B100  =   false;
//      n.B500  =   false;
//      n.B1000  =   false;
//      n.B5000  =   false;
//      c.EnableBillTypes(n);

//      c.Poll();
}

void Dlg::message(int code, const QString &msg)
{
//    if (code == 235) {
//        c.Reset();
//        startReceiveMoney();
//    }
    switch (code) {
    case 217:
        emit c_canPollingLoop(false);
        startReceiveMoney();
        break;
    }

    qDebug() << QString("%1: %2").arg(code).arg(msg);
}

void Dlg::deleteFile(bool error, const QString &data)
{
    if (error) {
        qDebug() << "Error" << data;
    }
    qDebug() << data;
    QFile f(sender()->property("file").toString());
    f.remove();
    QFile qr(sender()->property("qr").toString());
    qr.remove();
    sender()->deleteLater();
}

void Dlg::bill(WORD sum, bool canLoop)
{
    addCash(sum);
    FSum += sum;
    FRemain -= sum;
    if (FRemain < 0) {
        FRemain = 0;
    }
    emit totalSum(FSum, FNeeded, FRemain);
}

void Dlg::c_endPolling()
{
    Nominal n;
    n.B10   =   false;
    n.B50   =   false;
    n.B100  =   false;
    n.B500  =   false;
    n.B1000  =   false;
    n.B5000  =   false;
    emit c_enableBillTypes(n);
    emit c_poll();
}

void Dlg::timeout()
{
    fUserPageTimeout++;
}

QWidget *Dlg::addWidget(QWidget *w)
{
    QMutexLocker ml(&fMutex);
    if (fCurrentWidget) {
        fCurrentWidget->deleteLater();
    }
    fCurrentWidget = w;
    ui->vl->addWidget(w);
    return w;
}

void Dlg::addCash(double cash)
{
    emit c_canPollingLoop(false);
    qDebug() << "BEFORE RESET";
    emit c_reset();
    qDebug() << "AFTER RESET";
    HttpRequest *hr = new HttpRequest(QString("https://%1/app/terminal/add_cash").arg(server), SLOT(addCashResponse(bool, QString)), this);
    hr->setHeader("Authorization", "Bearer " + _s.value("driver_token").toString());
    hr->setFormData("type", QString::number(fMode));
    hr->setFormData("cash", QString::number(cash, 'f', 2));
    hr->postRequest();
}

void Dlg::makePayment()
{
    makePayment(fMode, FSum);
}

void Dlg::resetData()
{
    fUserPageTimeout = 0;
    FSum = 0;
    FRemain = 0;
    FNeeded = 0;
    fMode = 0;
    fDeposit = 0;
    emit (c_canPollingLoop(false));
}

void Dlg::firstPage()
{
    _s.remove("driver_token");
    _s.remove("balance");
    resetData();
    addWidget(new wUsernamePassword(this));
}

void Dlg::userPage()
{
    resetData();
    addWidget(new wLoading());
    HttpRequest *hr = new HttpRequest(QString("https://%1/app/terminal/get_debts").arg(server), SLOT(debts(bool, QString)), this);
    hr->setHeader("Authorization", "Bearer " + _s.value("driver_token").toString());
    hr->getRequest();
}

void Dlg::makePayment(int mode, double cash)
{
    fMode = mode;
    QString route;
    switch (fMode) {
    case 1:
        route = "pay_balance";
        break;
    case 2:
        route = "pay_debt";
        break;
    case 3:
        route = "pay_waybill";
        break;
    }

    if (fMode == 2) {
        if (cash <= FDept) {
            double remain = FDept - cash;
            fDeposit = fDeposit > remain ? remain : fDeposit;
        }
        if (FDept > cash + fDeposit) {
            fMode = 1;
            route = "pay_balance";
        }
        if (fMode == 1) {
            fDeposit = 0;
        }
    }

    if (fMode == 3) {
        if (cash < FNeeded) {
            double remain = FNeeded - cash;
            fDeposit = fDeposit > remain ? remain : fDeposit;
        } else {
            fDeposit = 0;
        }
    }
    if (fDeposit < 0) {
        fDeposit = 0;
    }

    HttpRequest *hr = new HttpRequest(QString("https://%1/app/terminal/").arg(server) + route, SLOT(payResponse(bool, QString)), this);
    hr->setHeader("Authorization", "Bearer " + _s.value("driver_token").toString());
    hr->setFormData("cash", QString::number(cash, 'f', 2));
    hr->setFormData("days", QString::number(fWaybillSelected));
    if (fMode == 2 || fMode == 3) {
        hr->setFormData("deposit", QString::number(fDeposit, 'f', 2));
    }    
    hr->postRequest();
}

void Dlg::closeEvent(QCloseEvent *e)
{
    emit (c_canPollingLoop(false));
    QDialog::closeEvent(e);
}


