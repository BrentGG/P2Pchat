#include "p2pclient.h"

#include <QCoreApplication>
#include <QString>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    P2Pclient* client = new P2Pclient();
    /*while (1) {
        char message[500];
        gets(message);
        client->sendMsg(message);
    }*/

    return a.exec();
}
