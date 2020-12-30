#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <QNetworkAccessManager>

class HttpRequest : public QNetworkAccessManager
{
    Q_OBJECT
public:
    explicit HttpRequest(const QString &url, const char *slot, QObject *parent = nullptr);
    ~HttpRequest();
    void setHeader(const QString &name, const QString &data);
    void setFormData(const QString &name, const QString &data);
    void postRequest();
    void getRequest();

private:
    QString fUrl;
    QString fContentType;
    QByteArray fData;
    QMap<QString, QString> fHeader;
    int fRequestMethod;
    void startThread();

private slots:
    void start();
    void finished(QNetworkReply *reply);\

signals:
    void response(bool error, const QString &data);
    void done();

};

#endif // HTTPREQUEST_H
