#include "httprequest.h"
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QFileInfo>
#include <QFile>
#include <QThread>

#define boundary QString("----7d935033608e27d935033608e2")
#define rmGET 1
#define rmPOST 2

HttpRequest::HttpRequest(const QString &url, const char* slot, QObject *parent) :
    QNetworkAccessManager(parent),
    fUrl(url)
{
    fContentType = "multipart/form-data, boundary=" + boundary;
    connect(this, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
    connect(this, SIGNAL(response(bool,QString)), parent, slot);
}

HttpRequest::~HttpRequest()
{
    qDebug() << "~HttpRequest";
}

void HttpRequest::setHeader(const QString &name, const QString &data)
{
    fHeader[name] = data;
}

void HttpRequest::setFormData(const QString &name, const QString &data)
{
    fData.append("--" + boundary + "\r\n");
    fData.append("Content-Disposition: form-data; name=\"" + name + "\"\r\n\r\n");
    fData.append(data + "\r\n");
}

void HttpRequest::setFileName(const QString &name, const QString &path)
{
    fFiles[name] = path;
    QFile f(path);
    if (f.open(QIODevice::ReadOnly)) {
        QFileInfo fileInfo(f.fileName());
        fData.append("--" + boundary + "\r\n");
        fData.append("Content-Type: application/octet-stream\r\n");
        fData.append("Content-Length: " + QString::number(f.size()) + "\r\n");
        fData.append("Content-Disposition: form-data; filename=\"" + fileInfo.fileName() + "\"; name=\"" + name + "\"\r\n\r\n");
        fData.append(f.readAll());
        fData.append("\r\n");
        f.close();
    }
}

void HttpRequest::postRequest()
{
    qDebug() << fUrl;
    qDebug() << fHeader;
    qDebug() << fData;
    log("POST");
    log(fUrl);
    for (QMap<QString, QString>::const_iterator it = fHeader.begin(); it != fHeader.end(); it++) {
        log(it.key() + "=" + it.value());
    }
    log(fData);
    fRequestMethod = rmPOST;
    startThread();
}

void HttpRequest::getRequest()
{
    qDebug() << fUrl;
    qDebug() << fHeader;
    qDebug() << fData;
    log("GET");
    log(fUrl);
    for (QMap<QString, QString>::const_iterator it = fHeader.begin(); it != fHeader.end(); it++) {
        log(it.key() + "=" + it.value());
    }
    log(fData);
    fRequestMethod = rmGET;
    startThread();
}

void HttpRequest::startThread()
{
    QThread *t = new QThread();
    connect(t, SIGNAL(started()), this, SLOT(start()));
    connect(t, SIGNAL(finished()), t, SLOT(deleteLater()));
    connect(this, SIGNAL(done()), t, SLOT(quit()));
    setParent(nullptr);
    moveToThread(t);
    t->start();
}

void HttpRequest::log(const QString &l)
{
#ifdef QT_DEBUG
    qDebug() << l;
#endif
    QFile f("log.txt");
    if (f.open(QIODevice::Append)) {
        f.write(l.toUtf8());
        f.write("\r\n");
        f.close();
    }
}

void HttpRequest::start()
{
    QUrl url(fUrl);
    QNetworkRequest nr = QNetworkRequest(url);
    if (fUrl.contains("https://")) {
        QSslConfiguration sslConf = nr.sslConfiguration();
        sslConf.setProtocol(QSsl::TlsV1_2);
        sslConf.setPeerVerifyMode(QSslSocket::VerifyNone);
        //sslConf.setProtocol(QSsl::AnyProtocol);
        nr.setSslConfiguration(sslConf);
    }

    nr.setRawHeader("Content-Type", fContentType.toLatin1());
    //nr.setRawHeader("Content-Length", QString::number(fData.length()).toLatin1());
    //nr.setRawHeader("Cache-Control", "no-cache");
    nr.setRawHeader("Accept", "application/json");

    for (QMap<QString, QString>::const_iterator it = fHeader.begin(); it != fHeader.end(); it++) {
        nr.setRawHeader(it.key().toLatin1(), it.value().toLatin1());
    }
    qDebug() << "Start " << fUrl;
    fTimer.start();
    switch (fRequestMethod) {
    case rmGET:
        get(nr);
        break;
    case rmPOST:
        post(nr, fData);
        break;
    }
}

void HttpRequest::finished(QNetworkReply *reply)
{
    QByteArray data = reply->readAll();
    qDebug() << "End " << fUrl << fTimer.elapsed();
    log(data);
    if (reply->error() == QNetworkReply::NoError) {
        emit response(false, data);
    } else {
        QJsonParseError je;
        QJsonObject jo;
        QJsonDocument jd = QJsonDocument::fromJson(data, &je);
        if (je.error != QJsonParseError::NoError) {
            jo["server_error"] = QString(data);
        } else {
            jo = jd.object();
        }
        jo["error_code"] =reply->error();
        jo["error_string"] = reply->errorString();
        log(QString::number(reply->error()));
        log(jo["error_string"].toString());
        emit response(true, QJsonDocument(jo).toJson());
    }
    emit done();
}
