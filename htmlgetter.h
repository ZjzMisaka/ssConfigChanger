#ifndef HTMLGETTER_H
#define HTMLGETTER_H

#include <QObject>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtCore>

class HtmlGetter : public QObject
{
    Q_OBJECT
public:
    explicit HtmlGetter(QObject *parent = nullptr);
    static QString getHtml(QString url);

signals:

public slots:
};

#endif // HTMLGETTER_H
