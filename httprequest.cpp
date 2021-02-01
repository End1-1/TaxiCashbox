#include "httprequest.h"
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
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

void HttpRequest::postRequest()
{
    qDebug() << fUrl;
    qDebug() << fHeader;
    qDebug() << fData;
    fRequestMethod = rmPOST;
    startThread();
}

void HttpRequest::getRequest()
{
    qDebug() << fUrl;
    qDebug() << fHeader;
    qDebug() << fData;
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

void HttpRequest::start()
{
    QUrl url(fUrl);;
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

    for (QMap<QString, QString>::const_iterator it = fHeader.begin(); it != fHeader.end(); it++)
        nr.setRawHeader(it.key().toLatin1(), it.value().toLatin1());
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
        emit response(true, QJsonDocument(jo).toJson());
    }
    emit done();
}
