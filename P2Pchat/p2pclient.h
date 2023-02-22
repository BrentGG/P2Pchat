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
    QTcpSocket* getFirstConn();

signals:
    void msgSent(QString msg);
    void msgRecd(QTcpSocket* peer, QString msg);
    void newConn(QTcpSocket* peer);
    void failedConn(QString addrAndPort);
    void disConn(QString addrAndPort);

private:
    QTcpServer* server;
    QList<QTcpSocket*> peers;
    QList<QString> peersAddrAndPort;

    void init();
    void loop();

    QString peerListToStr(QTcpSocket* target);
    void strToPeerList(QString str);
    bool addPeerToList(QTcpSocket* peer);

private slots:
    void newConnection();
    void read();
    void peerDisconnected();
};

#endif // P2PCLIENT_H
