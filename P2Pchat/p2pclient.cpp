#include "p2pclient.h"

#include <iostream>

P2Pclient::P2Pclient()
{
    init();
    //loop();
}

P2Pclient::P2Pclient(QString addr, unsigned short port)
{
    init();
    socket->connectToHost(addr, port);
    peers.append(socket->peerAddress().toString());
    socket->waitForConnected();
    std::cout << "Connected" << std::endl;
    socket->close();
    //loop();
}

void P2Pclient::init()
{
    socket = new QTcpSocket(this);

    server = new QTcpServer(this);
    connect(server, SIGNAL(newConnection()), this, SLOT(newConn()));
    if(!server->listen(QHostAddress::Any, 24042))
        qDebug() << "Server could not start";
    else
        qDebug() << "Server started!";
}

void P2Pclient::loop()
{
    while (1) {
        char message[500];
        gets(message);
        for(QString a : peers) {
            socket->connectToHost(a, 24042);
            socket->write(QString(message).toUtf8());
        }
    }
}

void P2Pclient::newConn()
{
    std::cout << "new connection" << std::endl;
    QTcpSocket *s = server->nextPendingConnection();

    if(!peers.contains(s->peerAddress().toString())) { // new peer
        std::cout << "Added peer to list" << std::endl;
        peers.append(s->peerAddress().toString());
        for(int i = 0; i < peers.size(); ++i)
            std::cout << peers[i].toStdString() << std::endl;
    }
    else { // existing peer
        QString data = QString::fromUtf8(s->readAll());
        std::cout << "Received data: " << data.toStdString() << std::endl;
    }

    s->close();
}

void P2Pclient::sendMsg(QString msg)
{
    for(QString a : peers) {
        socket->connectToHost(a, 24042);
        socket->write(QString(msg).toUtf8());
    }
}
