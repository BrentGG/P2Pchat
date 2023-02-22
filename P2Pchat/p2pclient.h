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
    QList<QTcpSocket*> getPeers();

    enum TYPE {
        SUCCESS,
        NEUTRAL,
        FAIL
    };

signals:
    void msgSent(QString msg);
    void msgRecd(QTcpSocket* peer, QString msg, QString name);
    void newConn(QTcpSocket* peer);
    void failedConn(QString addrAndPort);
    void disConn(QString addrAndPort);
    void print(QString text, TYPE type);

private:
    QTcpServer* server;
    QList<QTcpSocket*> peers;
    QList<QString> peersAddrAndPort;
    QList<QString> names;
    QList<bool> show;
    QString ownIp;

    void init();
    void loop();

    QString peerListToStr(QTcpSocket* target);
    void strToPeerList(QString str);
    bool addPeerToList(QTcpSocket* peer);
    void handleCommand(QString command);

private slots:
    void newConnection();
    void read();
    void peerDisconnected();
};

#endif // P2PCLIENT_H
