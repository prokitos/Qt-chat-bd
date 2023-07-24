#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QTime>
#include <QSqlDatabase>     // база данных
#include <QDebug>           // вывод сообщений в консоль отладки
#include <QSqlQuery>        // запросы

class server : public QTcpServer
{
    Q_OBJECT
public:
    server();
    QTcpSocket *socket;
    QVector<int> verifyAddress;
    QVector<QString> verifyProfile;
private:
    QVector <QTcpSocket*> Sockets;
    QByteArray Data;
    void SendToClient(QString str, QString profile);
    quint16 nextBlockSize;

    QSqlDatabase db;
    QSqlQuery *query;

public slots:
    void incomingConnection(qintptr socketDescriptor);
    void slotReadyRead();
};

#endif // SERVER_H
