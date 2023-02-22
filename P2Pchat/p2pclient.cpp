#include "p2pclient.h"

#include <iostream>

P2Pclient::P2Pclient()
{
    init();
}

P2Pclient::P2Pclient(QString addr, unsigned short port)
{
    init();
    QTcpSocket* socket = new QTcpSocket(this);
    socket->connectToHost(addr, port);
    socket->waitForConnected();
    std::cout << "Connected to " << socket->peerAddress().toString().toStdString() << ":" << QString::number(socket->peerPort()).toStdString() << std::endl;
    socket->waitForReadyRead();
    addPeerToList(socket);
    strToPeerList(QString(socket->readAll()));
    connect(socket, &QTcpSocket::readyRead, this, &P2Pclient::read);
    connect(socket, &QTcpSocket::disconnected, this, &P2Pclient::peerDisconnected);
}

void P2Pclient::init()
{
    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, this, &P2Pclient::newConnection);
    if(!server->listen(QHostAddress::Any, 24042))
        std::cout << "Server could not start" << std::endl;
    else
        std::cout << "Server started" << std::endl;
}

void P2Pclient::newConnection()
{
    QTcpSocket *socket = server->nextPendingConnection();
    if (addPeerToList(socket)) {
        socket->write(peerListToStr(socket).toUtf8());
        connect(socket, &QTcpSocket::readyRead, this, &P2Pclient::read);
        connect(socket, &QTcpSocket::disconnected, this, &P2Pclient::peerDisconnected);
        std::cout << "Connected to " << socket->peerAddress().toString().toStdString() << ":" << QString::number(socket->peerPort()).toStdString() << std::endl;
        emit newConn(socket);
    }
}

void P2Pclient::read()
{
    for (QTcpSocket* peer : peers) {
        while (peer->bytesAvailable() > 0) {
            QString msg = QString::fromUtf8(peer->readAll());
            QStringList list = msg.split('\n');
            if (list.size() > 0 && list[0] != "NEWCON") {
                std::cout << "<" << QHostAddress(peer->peerAddress().toIPv4Address()).toString().toStdString() << ":" << QString::number(peer->peerPort()).toStdString() << "> " << msg.toStdString() << std::endl;
                emit msgRecd(peer, msg);
            }
        }
    }
}

void P2Pclient::peerDisconnected()
{
    QTcpSocket* peer = (QTcpSocket*)QObject::sender();
    std::cout << "Peer at " << QHostAddress(peer->peerAddress().toIPv4Address()).toString().toStdString() << ":" << QString::number(peer->peerPort()).toStdString() << " disconnected" << std::endl;
    for (int i = 0; i < peers.size(); ++i) {
        if (peer == peers[i]) {
            emit disConn(peersAddrAndPort[i]);
            peers.remove(i);
            peersAddrAndPort.remove(i);
            break;
        }
    }
}

QString P2Pclient::peerListToStr(QTcpSocket* target)
{
    QString str = "NEWCON\n";
    for(QTcpSocket* peer : peers) {
        if (peer->state() == QTcpSocket::ConnectedState && peer != target) {
            str.append(QHostAddress(peer->peerAddress().toIPv4Address()).toString());
            str.append(":24042\n");
        }
    }
    return str;
}

void P2Pclient::strToPeerList(QString str)
{
    QStringList splitStr = str.split('\n');
    if (splitStr.size() > 2) {
        for(int i = 1; i < splitStr.size() - 1; ++i) {
            QTcpSocket *socket = new QTcpSocket(this);
            QStringList addrAndPort = splitStr[i].split(':');
            if (addrAndPort[0] != QHostAddress(socket->localAddress().toIPv4Address()).toString()) {
                socket->connectToHost(addrAndPort[0], addrAndPort[1].toUShort());
                socket->waitForConnected();
                socket->waitForReadyRead();
                if (socket->state() == QTcpSocket::ConnectedState) {
                    if (addPeerToList(socket)) {
                        connect(socket, &QTcpSocket::readyRead, this, &P2Pclient::read);
                        connect(socket, &QTcpSocket::disconnected, this, &P2Pclient::peerDisconnected);
                        std::cout << "Connected to " << splitStr[i].toStdString() << std::endl;
                        emit newConn(socket);
                    }
                }
                else {
                    std::cout << "Failed to connect to " << splitStr[i].toStdString() << std::endl;
                    emit failedConn(splitStr[i]);
                }
            }
        }
    }
}

bool P2Pclient::addPeerToList(QTcpSocket *peer)
{
    QString addrAndPort = QHostAddress(peer->peerAddress().toIPv4Address()).toString() + ":" + QString::number(peer->peerPort());
    if (!peersAddrAndPort.contains(addrAndPort)) {
        peers.append(peer);
        peersAddrAndPort.append(addrAndPort);
        return true;
    }
    return false;
}

void P2Pclient::sendMsg(QString msg)
{
    for(QTcpSocket* peer : peers) {
        peer->write(QString(msg).toUtf8());
    }
    std::cout << "Message sent: " << msg.toStdString() << std::endl;
    emit msgSent(msg);
}

QTcpSocket *P2Pclient::getFirstConn()
{
    return peers.size() > 0 ? peers[0] : nullptr;
}
