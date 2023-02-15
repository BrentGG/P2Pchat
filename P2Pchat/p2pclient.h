#ifndef P2PCLIENT_H
#define P2PCLIENT_H

#include <QTcpSocket>
#include <QTcpServer>
#include <QObject>
#include <QList>

class P2Pclient : public QObject
{
    Q_OBJECT
public:
    P2Pclient();
    P2Pclient(QString addr, unsigned short port);

    void sendMsg(QString msg);

private:
    QTcpSocket* socket;
    QTcpServer* server;

    QList<QString> peers;

    QString msg;

    void init();
    void loop();

signals:

private slots:
    void newConn();
};

#endif // P2PCLIENT_H
