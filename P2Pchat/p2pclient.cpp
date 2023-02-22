#include "p2pclient.h"

#include <iostream>
#include <QNetworkInterface>

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
    ownIp = QHostAddress(socket->localAddress().toIPv4Address()).toString();
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
    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    for (const QHostAddress &address: QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost)
             ownIp = QHostAddress(address.toIPv4Address()).toString();
    }
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
    for (int i = 0; i < peers.size(); ++i) {
        while (peers[i]->bytesAvailable() > 0) {
            QString msg = QString::fromUtf8(peers[i]->readAll());
            QStringList list = msg.split('\n');
            if (!show[i])
                emit print("Message by " + peersAddrAndPort[i] + " blocked", TYPE::NEUTRAL);
            else if (list.size() > 0 && list[0] != "NEWCON") {
                std::cout << "<" << QHostAddress(peers[i]->peerAddress().toIPv4Address()).toString().toStdString() << ":" << QString::number(peers[i]->peerPort()).toStdString() << "> " << msg.toStdString() << std::endl;
                emit msgRecd(peers[i], msg, names[i]);
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
            names.remove(i);
            show.remove(i);
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
        names.append("");
        show.append(true);
        return true;
    }
    return false;
}

void P2Pclient::handleCommand(QString command)
{
    QStringList commandSplit = command.split(' ');
    if (commandSplit[0] == "\\help")
        emit print("Available commands:\n\\myip", TYPE::NEUTRAL);
    else if (commandSplit[0] == "\\myip")
        emit print("Your IP: " + ownIp, TYPE::NEUTRAL);
    else if (commandSplit[0] == "\\setname" && commandSplit.size() > 2) {
        for (int i = 0; i < peers.size(); ++i) {
            if (peersAddrAndPort[i] == commandSplit[1] || (names[i] != "" && names[i] == commandSplit[1])) {
                names[i] = commandSplit[2];
                emit print("Name of " + commandSplit[1] + " set to " + commandSplit[2], TYPE::NEUTRAL);
                break;
            }
        }
    }
    else if (commandSplit[0] == "\\block" && commandSplit.size() > 1) {
        for (int i = 0; i < peers.size(); ++i) {
            if (peersAddrAndPort[i] == commandSplit[1] || (names[i] != "" && names[i] == commandSplit[1])) {
                show[i] = false;
                emit print("Blocked " + commandSplit[1], TYPE::NEUTRAL);
                break;
            }
        }
    }
    else if (commandSplit[0] == "\\unblock" && commandSplit.size() > 1) {
        for (int i = 0; i < peers.size(); ++i) {
            if (peersAddrAndPort[i] == commandSplit[1] || (names[i] != "" && names[i] == commandSplit[1])) {
                show[i] = true;
                emit print("Unblocked " + commandSplit[1], TYPE::NEUTRAL);
                break;
            }
        }
    }
    else
        emit print("Command not recognized", TYPE::FAIL);
}

void P2Pclient::sendMsg(QString msg)
{
    if (msg[0] != '\\') {
        for(QTcpSocket* peer : peers) {
            peer->write(QString(msg).toUtf8());
        }
        std::cout << "Message sent: " << msg.toStdString() << std::endl;
        emit msgSent(msg);
    }
    else
        handleCommand(msg);
}

QList<QTcpSocket*> P2Pclient::getPeers()
{
    return peers;
}
