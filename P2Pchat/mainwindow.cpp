#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent, QString addr, quint16 port): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    if (addr != "")
        client = new P2Pclient(addr, port);
    else
        client = new P2Pclient();
    connect(client, &P2Pclient::msgSent, this, &MainWindow::dispMsgSent);
    connect(client, &P2Pclient::msgRecd, this, &MainWindow::dispMsgRecd);
    connect(client, &P2Pclient::newConn, this, &MainWindow::dispNewConn);
    connect(client, &P2Pclient::failedConn, this, &MainWindow::dispFailedConn);
    connect(client, &P2Pclient::disConn, this, &MainWindow::dispDisConn);
    connect(client, &P2Pclient::print, this, &MainWindow::systemPrint);

    QWidget* centralWidget = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout();
    recdField = new QTextEdit();
    recdField->setReadOnly(true);
    mainLayout->addWidget(recdField);
    QHBoxLayout* sendLayout = new QHBoxLayout();
    sendField = new QLineEdit();
    sendButton = new QPushButton("&Send");
    sendLayout->addWidget(sendField);
    sendLayout->addWidget(sendButton);
    mainLayout->addLayout(sendLayout);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
    setWindowTitle("P2Pchat");

    connect(sendButton, &QPushButton::clicked, this, &MainWindow::sendButtonPressed);
    connect(sendField, &QLineEdit::returnPressed, this, &MainWindow::sendButtonPressed);

    QTimer::singleShot(0, this, &MainWindow::dispFirstConn);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::dispMsgSent(QString msg)
{
    recdField->setTextColor(QColor::fromRgb(0, 0, 0));
    recdField->append("<You> " + msg);
}

void MainWindow::dispMsgRecd(QTcpSocket *peer, QString msg, QString name)
{
    recdField->setTextColor(QColor::fromRgb(0, 0, 0));
    QString dispName = name == "" ? QHostAddress(peer->peerAddress().toIPv4Address()).toString() + ":" + QString::number(peer->peerPort()) : name;
    recdField->append("<" + dispName + "> " + msg);
}

void MainWindow::dispNewConn(QTcpSocket *peer)
{
    recdField->setTextColor(QColor::fromRgb(0, 255, 0));
    recdField->append("<System> connected to " + QHostAddress(peer->peerAddress().toIPv4Address()).toString() + ":" + QString::number(peer->peerPort()));
}

void MainWindow::dispFailedConn(QString addrAndPort)
{
    recdField->setTextColor(QColor::fromRgb(255, 0, 0));
    recdField->append("<System> failed to connect to " + addrAndPort);
}

void MainWindow::sendButtonPressed()
{
    QString msg = sendField->text();
    if (!msg.isEmpty()) {
        client->sendMsg(msg);
        sendField->clear();
    }
}

void MainWindow::dispFirstConn()
{
    QList<QTcpSocket*> peers = client->getPeers();
    recdField->setTextColor(QColor::fromRgb(0, 255, 0));
    if (peers.size() <= 0)
        recdField->append("<System> waiting for peers to connect");
    else {
        for (QTcpSocket* peer : peers)
            recdField->append("<System> connected to " + QHostAddress(peer->peerAddress().toIPv4Address()).toString() + ":" + QString::number(peer->peerPort()));
    }
}

void MainWindow::dispDisConn(QString addrAndPort)
{
    recdField->setTextColor(QColor::fromRgb(255, 0, 0));
    recdField->append("<System> " + addrAndPort + " disconnected");
}

void MainWindow::systemPrint(QString text, P2Pclient::TYPE type)
{
    if (type == P2Pclient::NEUTRAL)
        recdField->setTextColor(QColor::fromRgb(0, 0, 255));
    else if (type == P2Pclient::SUCCESS)
        recdField->setTextColor(QColor::fromRgb(0, 255, 0));
    else if (type == P2Pclient::FAIL)
        recdField->setTextColor(QColor::fromRgb(255, 0, 0));
    recdField->append("<System> " + text);
}

