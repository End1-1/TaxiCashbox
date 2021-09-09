#include "cashbox.h"
#include <QException>
#include <QDateTime>
#include <QMessageBox>
#include <QDebug>

CashBox::CashBox() :
    QObject(nullptr)
{

}

CashBox::~CashBox()
{
    closeComPort();
}

bool CashBox::openComPort(char *port)
{
    wchar_t *wport = new wchar_t[255];
    memset(reinterpret_cast<void *>(wport), 0, sizeof(wchar_t) * 255);
    mbstowcs(wport, port, strlen(port));

    FComFile = CreateFile(wport, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (!FComFile)
        return false;

    SetupComm(FComFile, static_cast<DWORD>(2048), static_cast<DWORD>(2048));

    DCB dcb;
    memset(reinterpret_cast<void *>(&dcb), 0, sizeof(DCB));
    bool fSuccess = GetCommState(FComFile, &dcb);
    if(!fSuccess)
        return false;

    dcb.BaudRate = CBR_9600;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;

    fSuccess = SetCommState(FComFile, &dcb);
    if (!fSuccess)
        return false;

    COMMTIMEOUTS ct;
    fSuccess = GetCommTimeouts(FComFile, &ct);
    if (!fSuccess)
        return false;
    ct.ReadIntervalTimeout = 100;
    ct.ReadTotalTimeoutConstant = 500;
    ct.ReadTotalTimeoutMultiplier = 0;
    ct.WriteTotalTimeoutConstant = 2000;
    ct.WriteTotalTimeoutMultiplier = 60;

    fSuccess = SetCommTimeouts(FComFile, &ct);
    FComConnected = fSuccess;

    COMMTIMEOUTS t;
    t.ReadIntervalTimeout = 400;
    t.ReadTotalTimeoutMultiplier = 0;
    t.ReadTotalTimeoutConstant = 400;
    t.WriteTotalTimeoutMultiplier = 0;
    t.WriteTotalTimeoutConstant = 400;
    SetCommTimeouts(FComFile, &t);

    return fSuccess;
}

void CashBox::closeComPort()
{
    if (FComFile != INVALID_HANDLE_VALUE) {
        CloseHandle(FComFile);
        FComFile = INVALID_HANDLE_VALUE;
    }
    FComConnected = false;
}

void CashBox::processCommand()
{
    DWORD BytesWritten;
    DWORD BytesRead;
    LPDWORD Errs;
    LPCOMSTAT ComStat;

    ClearCommError(FComFile, Errs, ComStat);       //ñ÷èòûâàåì ñîñòîÿíèå ïîðòà
    if (ComStat->cbInQue > 0) {
        PurgeComm(FComFile, PURGE_TXCLEAR | PURGE_RXCLEAR);
        errorLog("Íå óäàëîñü î÷èñòèòü ïîðò");
    }

    if (!WriteFile(FComFile, FCommand, FLengthCommand, &BytesWritten, nullptr)) {
        errorLog("Íå óäàëîñü çàïèñàòü êîìàíäó â ïîðò");
    }

    if (BytesWritten != FLengthCommand) {
        errorLog("Íå óäàëîñü çàïèñàòü êîìàíäó â ïîðò äî êîíöà");
    }

    if (!ClearCommError(FComFile, Errs, ComStat)) {
        errorLog("Íå óäàëîñü ñáðîñèòü îøèáêè COM ïîðòà");
    }

    ClearAnswer();
    if (!ReadFile(FComFile, FAnswer, sizeof(FAnswer), &BytesRead, nullptr)) {
        errorLog("Íå óäàëîñü ïðî÷èòàòü îòâåò èç ïîðòà");
    } else {
        FLengthAnswer = BytesRead;
        ParseAnswer();
    }
}


void CashBox::ClearAnswer()
{
    memset(FAnswer, 0, sizeof(FAnswer));
    FLengthAnswer = 0;
}

void CashBox::ClearData()
{
    memset(FData, 0, sizeof(FData));
    FLengthData = 0;
}

void CashBox::ClearCommand()
{
    memset(FCommand, 0, sizeof(FCommand));
    FLengthCommand = 0;
}

void CashBox::ParseAnswer()
{
    BYTE CRC16[2];
    WORD CRCWord;
    ClearData();
    if (FLengthAnswer >= 6) {
        CRCWord = GetCRC16(FAnswer, FLengthAnswer-2);
        CopyMemory(CRC16, &CRCWord, sizeof(CRCWord));
        if ((CRC16[0] == FAnswer[FLengthAnswer-2]) && (CRC16[1] == FAnswer[FLengthAnswer-1])) {
            CopyMemory(FData, &FAnswer[3], FLengthAnswer - 5);
            FLengthData = FLengthAnswer - 5;
        } else {
            errorLog("Не верный CRC ответа.");
        }
    } else {
        //errorLog("Не полный ответ");
    }
}

bool CashBox::ProcessCommand()
{
    DWORD BytesWritten;
    DWORD BytesRead;
    LPDWORD Errs;
    LPCOMSTAT ComStat;

    ClearCommError(FComFile, Errs, ComStat);
    if (Errs != 0) {
        QMessageBox::critical(0, "ComPort Error", QString("GetLastError() returned #%1").arg(GetLastError()));
        return false;
    }
    if (ComStat) {
        if (ComStat->cbInQue > 0) {
            if (!PurgeComm(FComFile, PURGE_TXCLEAR | PURGE_RXCLEAR)) {
                errorLog("Не удалось очистить порт");
                return false;
            }
         }
    }


    if (!WriteFile(FComFile, FCommand, FLengthCommand, &BytesWritten, nullptr)) {
        errorLog("Не удалось записать команду в порт");
        return false;
    }

    if (BytesWritten != FLengthCommand) {
        errorLog("Не удалось записать команду в порт до конца");
        return false;
    }

    if (!ClearCommError(FComFile, Errs, ComStat)) {
        errorLog("Не удалось сбросить ошибки COM порта");
        return false;
    }

    ClearAnswer();
    DWORD answerSize = sizeof(FAnswer);
    if (!ReadFile(FComFile, FAnswer, answerSize, &BytesRead, nullptr)) {
        errorLog("Не удалось прочитать ответ из порта");
        return false;
    } else {
        if (BytesRead == 0) {
            qDebug() << GetLastError();
        }
        FLengthAnswer = BytesRead;
        ParseAnswer();
    }
    return true;
}

void CashBox::errorLog(const QString &msg)
{
    //QMessageBox::critical(0, "ERROR", msg);
#ifndef QT_DEBUG
    exit(0);
#endif
}

bool CashBox::EnableBillTypes(Nominal n)
{
    BYTE BillTypesByte;
    BYTE b[6];
    if (!FComConnected) {
        errorLog("COM порт закрыт, выполнение команды RESET не возможно");
    }

    BillTypesByte = 0;

  // Óñòàíîâèì áèòû êóïþð
  if (n.B10) {
      BillTypesByte = BillTypesByte+B10;
  }
  if (n.B50) {
      BillTypesByte = BillTypesByte+B50;
  }
  if (n.B100) {
      BillTypesByte = BillTypesByte+B100;
  }
  if (n.B500) {
      BillTypesByte = BillTypesByte+B500;
  }
  if (n.B1000) {
      BillTypesByte = BillTypesByte+B1000;
  }
  if (n.B5000) {
      BillTypesByte = BillTypesByte+B5000;
  }

  b[0] = 0;
  b[1] = 0;
  b[2] = BillTypesByte;
  b[3] = 0;
  b[4] = 0;
  b[5] = 0;
  //SendPacket($34,[0,0,BillTypesByte,0,0,0]);
  SendPacket(0x34, b, 6);
  emit ProcessMessage(206, "->ENABLE BILL TYPES");
  ProcessCommand();

  if (FData[0] == 0xFF) {
    emit ProcessMessage(202, "<-NSC");
    errorLog("Ïîëó÷åí îòðèöàòåëüíûé îòâåò (NAK)");
  }

  if (FData[0] == 0x00) {
    emit ProcessMessage(203, "<-ASC");
  }
// except
//   on E:Exception do begin
//     ProcessMessage(104,E.Message);
//     result:=false;
//   end;
// end;
 return true;
}

void CashBox::SendPacket(BYTE Command, LPBYTE Data, BYTE DataLength)
{
//procedure TCashCodeBillValidatorCCNET.SendPacket(Command: Byte; Data: array of Byte);
    BYTE CRC16[2];
    WORD CRCWord;
    ClearCommand();

    FLengthCommand = 6 + DataLength; //FLengthCommand =6 + Length(Data); ????

    FCommand[0] = 0x02; // Ñèíõðîíèçàöèîííûé áàéò
    FCommand[1] = 0x03; // Àäðåññ âàëèäàòîðà
    FCommand[2] = FLengthCommand; //Äëèíà âñåãî ïàêåòà âêëþ÷àÿ CRC
    FCommand[3] = Command;   // Êîìàíäà

    if (DataLength != 0) {
        CopyMemory(&FCommand[4], Data, DataLength);
    }

    CRCWord = GetCRC16(FCommand, FLengthCommand - 2);   // Ïîëó÷èì CRC
    CopyMemory(CRC16, &CRCWord, 2);             // Ðàçàáüåì CRC íà áàéòû

    FCommand[FLengthCommand-2] = CRC16[0];           // Ïîäïèøåì íàøó êîìàíäó
    FCommand[FLengthCommand-1] = CRC16[1];
}

WORD CashBox::GetCRC16(LPBYTE InData, WORD DataLng)
{
    WORD i,  TmpCRC;
    BYTE j;
    WORD result = 0;
    for ( i= 0; i < DataLng; i++) {
        TmpCRC = result ^ InData[i];
        for (j = 0; j < 8; j++) {
            if ((TmpCRC & 0x0001) != 0) {
                TmpCRC = TmpCRC >> 1;
                TmpCRC = TmpCRC ^ POLYNOM;
            } else {
                TmpCRC = TmpCRC >> 1;
            }
        }
        result = TmpCRC;
    }
    return result;
}

bool CashBox::Poll()
{
    try {
        if (!FComConnected) {
            errorLog("COM порт закрыт, выполнение команды RESET не возможно");
            return false;
        }
        SendPacket(0x33, nullptr, 0);
        emit ProcessMessage(201, "->POLL");
        ProcessCommand();
    } catch (...) {
        ProcessMessage(110, "XUYYYYAAA");
        return false;
    }
    return true;
}

WORD CashBox::PollingLoop(WORD Sum, DWORD TimeLoop)
{
    QDateTime StartTime = QDateTime::currentDateTime();
    BYTE FirstByte, SecondByte;
    WORD BillNominal;
    WORD result = 0;

    FCanPollingLoop = true;
    while (FCanPollingLoop) {
        if (Poll()) {
            FirstByte = FData[0];
            SecondByte = FData[1];

            switch (FirstByte) {
            case 0x10:
            case 0x11:
            case 0x12:
                emit ProcessMessage(213, "Включение питания после команд");
                FCanPollingLoop = false;
                break;
            case 0x13:
                emit ProcessMessage(214,"Инициализация");
                break;
            case 0x14:
                emit ProcessMessage(215,"Ожидание приема купюры");
                break;
            case 0x15:
                emit ProcessMessage(216,"Акцепт");
                break;
            case 0x19:
                emit ProcessMessage(217,"Недоступен, ожидаю инициализации");
                break;
            case 0x41:
                emit ProcessMessage(218,"Полная кассета");
                Reset();
                FCanPollingLoop = false;
                break;
            case 0x42:
                emit ProcessMessage(219,"Êàññåòà îòñóòñòâóåò");
                Reset();
                FCanPollingLoop = false;
                break;
            case 0x43:
                emit ProcessMessage(220,"Çàìÿëî êóïþðó");
                Reset();
                FCanPollingLoop = false;
                break;
            case 0x44:
                emit ProcessMessage(221,"Çàìÿëî êàñåòó 0_o");
                Reset();
                FCanPollingLoop = false;
                break;
            case 0x45:
                emit ProcessMessage(222,"ÊÀÐÀÓË !!!! ÆÓËÈÊÈ !!!");
                Reset();
                FCanPollingLoop = false;
                break;
            case 0x47:
                emit ProcessMessage(223,"Сбой оборудования");
                switch (SecondByte) {
                case 0x50:
                    emit ProcessMessage(224,"Stack_motor_falure");
                    break;
                case 0x51:
                    emit ProcessMessage(225,"Transport_speed_motor_falure");
                    break;
                case 0x52:
                    emit ProcessMessage(226,"Transport-motor_falure");
                    break;
                case 0x53:
                    emit ProcessMessage(227,"Aligning_motor_falure");
                    break;
                case 0x54:
                    emit ProcessMessage(228,"Initial_cassete_falure");
                    break;
                case 0x55:
                    emit ProcessMessage(229,"Optical_canal_falure");
                    break;
                case 0x56:
                    emit ProcessMessage(230,"Magnetical_canal_falure");
                    break;
                case 0x5F:
                    emit ProcessMessage(231,"Capacitance_canal_falure");
                    break;
                }
                break;
            case 0x1C:
                emit ProcessMessage(232,"Отказ от приема");
                switch (SecondByte) {
                case 0x60:
                    emit ProcessMessage(233,"Insertion_error");
                    break;
                case 0x61:
                    emit ProcessMessage(234,"Dielectric_error");
                    break;
                case 0x62:
                    emit ProcessMessage(235,"Previously_inserted_bill_remains_in_head");
                    break;
                case 0x63:
                    emit ProcessMessage(236,"Compensation__factor_error");
                    break;
                case 0x64:
                    emit ProcessMessage(237,"Bill_transport_error");
                    break;
                case 0x65:
                    emit ProcessMessage(238,"Identification_error");
                    break;
                case 0x66:
                    emit ProcessMessage(239,"Verification_error");
                    break;
                case 0x67:
                    emit ProcessMessage(240,"Optic_sensor_error");
                    break;
                case 0x68:
                    emit ProcessMessage(241,"Return_by_inhibit_error");
                    break;
                case 0x69:
                    emit ProcessMessage(242,"Capacistance_error");
                    break;
                case 0x6A:
                    emit ProcessMessage(243,"Operation_error");
                    break;
                case 0x6C:
                    emit ProcessMessage(244,"Length_error");
                    break;
                }
                break;
            case 0x80:
                emit ProcessMessage(245,"Äåïîíåò"); // Äîðàáàòûâàòü íå ñòàë, òàê êàê íå âèæó ïðàêòè÷åñêîãî ïðèìåíåíèÿ, â îáùåì êîìó íàäî òîò äîïèëèò
                break;
            case 0x81:
                emit ProcessMessage(246,"Укладка");
                switch (SecondByte) {
                case 2:
                    BillNominal =10;
                    break;
                case 3:
                    BillNominal =50;
                    break;
                case 4:
                    BillNominal =100;
                    break;
                case 5:
                    BillNominal =500;
                    break;
                case 6:
                    BillNominal =1000;
                    break;
                case 7:
                    BillNominal =5000;
                    break;
                }
                StartTime = QDateTime::currentDateTime();
                SendASC();
                result += BillNominal;
                emit PolingBill(BillNominal, FCanPollingLoop);
                if (result >=Sum) {
                    FCanPollingLoop = false;
                    emit ProcessMessage(247,"Набрали нужную сумму");
                }
                break;
            case 0x82:
                emit ProcessMessage(248,"Возврат купюры");
                break;
            }
            Sleep(100);
        }

        int diff = StartTime.msecsTo(QDateTime::currentDateTime()) / 1000;
        if (diff > TimeLoop) {
            FCanPollingLoop = false;
            emit ProcessMessage(249,"Завершаем работу по таймауту приема купюры");
        }
    }
    return result;
}

bool CashBox::SendASC()
{
    if (!FComConnected) {
        errorLog("COM порт закрыт, выполнение команды RESET не возможно");
        return false;
    }
    SendPacket(0x00, nullptr, 0);
    emit ProcessMessage(211, "->ASC");
    ProcessCommand();
    return true;
}

bool CashBox::Reset()
{
    if (!FComConnected) {
        errorLog("COM ïîðò çàêðûò, âûïîëíåíèå êîìàíäû RESET íå âîçìîæíî");
        return false;
    }
    SendPacket(0x30, nullptr, 0);
    emit ProcessMessage(204, "->RESET");
    ProcessCommand();
    if (FData[0] == 0xFF) {
        emit ProcessMessage(202, "<-NSC");
        errorLog("Ïîëó÷åí îòðèöàòåëüíûé îòâåò (NAK)");
    }
    if (FData[0] == 0x00) {
        emit ProcessMessage(203, "<-ASC");
    }
    return true;
}
