#ifndef MYSERVER_H
#define MYSERVER_H

#include <QTcpServer>
#include <QList>
#include <QTcpSocket>
#include <memory>
#include <QDataStream>
#include <QPixmap>
#include <QPair>
#include <vector>
#include <QVector>
#include <QThread>

#include "../client/config.h"
#include "socketmetadata.h"

class MyServer : public QTcpServer
{
    Q_OBJECT
public:
    MyServer(QObject* parent = nullptr);

signals:
    void videoSocketConnected(QTcpSocket *);
    void audioSocketConnected(QTcpSocket *);
    void stopAllWorkers();

private slots:
    void handleConnection();
    void getSocketType();
    void socketError(QAbstractSocket::SocketError error);

private:
    SocketList tmpList;
    MetaDataList tmpMetaData;

    QThread *videoThread;
    QThread *audioThread;

    void prepareVideoWorker();
    void prepareAudioWorker();

    bool readSize(QTcpSocket *socket, SocketMetaData *metaData);
    bool readData(QTcpSocket *socket, SocketMetaData *metaData, QString& resultStr);
    void prepareSocket(const QString &type, int i);
    void handleTmpSocketErr(int i, QTcpSocket* sock);
    void handleVideoSocketErr(QAbstractSocket::SocketError error, int i, QTcpSocket* socket);
};

#endif // MYSERVER_H
