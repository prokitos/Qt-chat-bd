#include "server.h"

// first run to create db
void createDB(QSqlDatabase db1)
{
    //db1 = QSqlDatabase::addDatabase("QSQLITE");
    //db1.setDatabaseName("./profiles.db");

    if(db1.open())
    {
        qDebug("create");
    }
    else
    {
        qDebug("noCreate");
    }

    QSqlQuery query;

    query.exec("CREATE TABLE account(login TEXT, password TEXT);");
    query.exec("INSERT INTO account VALUES ('user', '123456');");
    query.exec("INSERT INTO account VALUES ('admin', '654321');");

    db1.close();

}

// compare account in db
bool queryDB(QString login, QString password, QSqlDatabase db2)
{
    bool isApproved = false;

    if(db2.open())
    {
        qDebug("open");
    }
    else
    {
        qDebug("noOpen");
    }

    QSqlQuery query;
    query.exec("SELECT * FROM account;");

    while (query.next()) {
        QString name = query.value(0).toString();
        QString pass = query.value(1).toString();
        if(name == login && pass == password)
        {
            isApproved = true;
            qDebug() << "approved login!!!";
        }
    }

    db2.close();

    return isApproved;
}

server::server()
{
    //createDB(db);
    //queryDB();
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("./profiles.db");


    if(this->listen(QHostAddress::Any,2323))
    {
        qDebug() << "start";
    }
    else
    {
        qDebug() << "error";
    }
    nextBlockSize = 0;
}

void server::incomingConnection(qintptr socketDescriptor)
{
    socket = new QTcpSocket;
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket,&QTcpSocket::readyRead,this,&server::slotReadyRead);
    connect(socket,&QTcpSocket::disconnected,this,&server::deleteLater);

    Sockets.push_back(socket);
    qDebug() << "client coonected" << socketDescriptor;
}

void server::slotReadyRead()
{
    socket = (QTcpSocket*)sender();
    QDataStream in (socket);
    QString userName = "";

    bool isLegal = false;   // profile approved or no

    // if this user is approved
    for(short i = 0; i < verifyAddress.length(); i++)
    {
        if(verifyAddress[i] == socket->socketDescriptor())
        {
            userName += "[";
            userName += verifyProfile[i];
            userName += "]";
            isLegal = true;
        }
    }

    in.setVersion(QDataStream::Qt_6_2);
    if(in.status() == QDataStream::Ok)
    {
        /*
        qDebug() << "read...";
        QString str;
        in >> str;
        qDebug() << str;
        SendToClient(str);
        */

        for(;;)
        {
            if(nextBlockSize == 0)
            {
                if(socket->bytesAvailable() < 2)
                {
                    break;
                }
                in >> nextBlockSize;
                qDebug() << "nextblocksize = " << nextBlockSize;
            }
            if(socket->bytesAvailable() < nextBlockSize)
            {
                break;
            }
            QString str, temp;
            QTime time;

            in >> time >> str;
            nextBlockSize = 0;

            qDebug() << str;

            // if no access to server data, try to login
            if(isLegal == false)
            {
                QString login = "";
                QString password = "";

                bool isFirstPiece = true;
                for(int i = 0; i < str.length(); i++)
                {
                    if(str[i] == ' ')
                    {
                        isFirstPiece = false;
                    }
                    if(isFirstPiece == true)
                    {
                        login += str[i];
                    }
                    if(isFirstPiece == false && ' ' != str[i])
                    {
                        password += str[i];
                    }
                }

                // compare login and password with db, and write verify socket
                bool autorization = queryDB(login,password, db);
                if(autorization == true)
                {
                    verifyAddress.push_back(socket->socketDescriptor());
                    verifyProfile.push_back(login);
                    SendToClient(login,"autorizated");
                }

                return;
            }
            SendToClient(str,userName);
            break;
        }
    }
    else
    {
        qDebug() << "DataStream error";
    }
}

void server::SendToClient(QString str, QString profile)
{
    Data.clear();
    QDataStream out(&Data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_2);
    out << quint16(0) << QTime::currentTime() << profile + " " + str;
    out.device()->seek(0);
    out << quint16(Data.size() - sizeof(quint16));
    //socket->write(Data);
    for (short i = 0; i < Sockets.size(); i++)
    {
        Sockets[i]->write(Data);
    }

}



