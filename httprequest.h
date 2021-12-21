#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <QNetworkAccessManager>
#include <QElapsedTimer>

class HttpRequest : public QNetworkAccessManager
{
    Q_OBJECT
public:
    explicit HttpRequest(const QString &url, const char *slot, QObject *parent = nullptr);
    ~HttpRequest();
    void setHeader(const QString &name, const QString &data);
    void setFormData(const QString &name, const QString &data);
    void setFileName(const QString &name, const QString &path);
    void postRequest();
    void getRequest();
    QByteArray fData;
    QString fContentType;

private:
    QString fUrl;
    QMap<QString, QString> fHeader;
    QMap<QString, QString> fFiles;
    int fRequestMethod;
    QElapsedTimer fTimer;
    void startThread();
    void log(const QString &l);

private slots:
    void start();
    void finished(QNetworkReply *reply);\

signals:
    void response(bool error, const QString &data);
    void done();

};

#endif // HTTPREQUEST_H
