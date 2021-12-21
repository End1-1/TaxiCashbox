#ifndef CASHBOX_H
#define CASHBOX_H

#include <QString>
#include <QObject>
#include <QTimer>
#include <windows.h>

const int POLYNOM = 0x08408,
        B10   =   4,  //00000100
        B50   =   8,  //00001000
        B100  =   16, //00010000
        B500  =   32, //00100000
        B1000 =   64, //01000000
        B5000 =   128;//10000000

struct Nominal {
    bool B10;
    bool B50;
    bool B100;
    bool B500;
    bool B1000;
    bool B5000;
};

class CashBox : public QObject
{
    Q_OBJECT

public:
    CashBox();
    bool FCanPollingLoop;
    ~CashBox();
    bool openComPort(char *port);
    void closeComPort();
    void processCommand();
    void ClearAnswer();
    void ClearData();
    void ClearCommand();
    void ParseAnswer();
    bool ProcessCommand();
    void errorLog(const QString &msg);
    bool EnableBillTypes(Nominal n);
    void SendPacket(BYTE Command, LPBYTE Data, BYTE DataLength);
    WORD GetCRC16(LPBYTE InData, WORD DataLng);
    bool Poll();
    WORD PollingLoop(WORD Sum, DWORD TimeLoop);
    bool SendASC();
    bool Reset();

public slots:
    void reset();
    void canPollingLoop(bool v);
    void enableBillTypes(Nominal n);
    void pollingLoop(WORD sum, DWORD TimeLoop);
    void poll();

private:
    HANDLE FComFile;
    BYTE FData[255];
    DWORD FLengthData;
    BYTE FCommand[255];
    DWORD FLengthCommand;
    BYTE FAnswer[255];
    DWORD FLengthAnswer;
    bool FComConnected;
    QTimer fTimer;

private slots:
    void timeout();

signals:
    void ProcessMessage(int CodeMess, const QString &msg);
    void PolingBill(WORD Bill, bool CanLoop);
    void EndPolling();
};

#endif // CASHBOX_H
