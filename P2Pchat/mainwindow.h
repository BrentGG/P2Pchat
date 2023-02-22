#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "p2pclient.h"

#include <QMainWindow>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr, QString addr = "", quint16 port = 0);
    ~MainWindow();

private slots:
    void dispMsgSent(QString msg);
    void dispMsgRecd(QTcpSocket* peer, QString msg, QString name);
    void dispNewConn(QTcpSocket* peer);
    void dispFailedConn(QString addrAndPort);
    void sendButtonPressed();
    void dispFirstConn();
    void dispDisConn(QString addrAndPort);
    void systemPrint(QString text, P2Pclient::TYPE type);

private:
    Ui::MainWindow *ui;

    P2Pclient* client;

    QLineEdit* sendField;
    QTextEdit* recdField;
    QPushButton* sendButton;
};
#endif // MAINWINDOW_H
