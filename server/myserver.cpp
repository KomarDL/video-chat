#include "myserver.h"
#include "videoworker.h"
#include "audioworker.h"
#include "dialog.h"

MyServer::MyServer(QObject *parent):
    QTcpServer(parent),
    videoThread(new QThread),
    audioThread(new QThread)
{
    connect(this, &QTcpServer::newConnection, this, &MyServer::handleConnection);
    prepareVideoWorker();
    prepareAudioWorker();
    audioThread->start();
    videoThread->start();
}

void MyServer::handleConnection()
{
    qDebug() << "connected";
    //get connected socket
    auto tmpSocket = this->nextPendingConnection();

    //add socket and metadata for socket to temporally lists
    tmpList.push_back(tmpSocket);
    tmpMetaData.push_back(new SocketMetaData);

    connect(tmpSocket, &QTcpSocket::readyRead, this, &MyServer::getSocketType);
}

void MyServer::getSocketType()
{
    auto senderSocket = dynamic_cast<QTcpSocket*>(sender());
    auto i = tmpList.indexOf(senderSocket);

    if (i != -1) {
        qInfo() << "reading";
        if (tmpMetaData[i]->isSize) {
            qInfo() << "reading size";
            if (!readSize(senderSocket, tmpMetaData[i])) {
                qInfo() << "size didn't read";
                return;
            }
        }
        QString result;
        qInfo() << "reading data";
        tmpMetaData[i]->isSize = readData(senderSocket, tmpMetaData[i], result);
        if (!result.isEmpty()) {
            qInfo() << "data was read";
            prepareSocket(result, i);
        }
    }
}

void MyServer::socketError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    auto senderSocket = dynamic_cast<QTcpSocket*>(sender());
    qInfo() << senderSocket->errorString();
    senderSocket->disconnectFromHost();
    auto i = tmpList.indexOf(senderSocket);
    tmpList.removeAt(i);
    delete tmpMetaData.takeAt(i);
}

void MyServer::prepareVideoWorker()
{
    //create worker
    auto videoWorker = new VideoWorker;
    videoWorker->moveToThread(videoThread);
    //start worker when thread started
    connect(videoThread, &QThread::started, videoWorker, &AbstractWorker::process);
    //delete objects when thread finished work
    connect(videoWorker, &AbstractWorker::finished, videoWorker, &QObject::deleteLater);
    connect(videoThread, &QThread::finished, videoThread, &QObject::deleteLater);
    //stop worker thread when it necessary
    connect(this, &MyServer::stopAllWorkers, videoWorker, &AbstractWorker::stop);
    //move socket to video worker thread
    connect(this, &MyServer::videoSocketConnected, videoWorker, &AbstractWorker::getSocket);
}

void MyServer::prepareAudioWorker()
{
    //create worker
    auto audioWorker = new AudioWorker;
    audioWorker->moveToThread(audioThread);
    //start worker when thread started
    connect(audioThread, &QThread::started, audioWorker, &AbstractWorker::process);
    //delete objects when thread finished work
    connect(audioWorker, &AbstractWorker::finished, audioWorker, &QObject::deleteLater);
    connect(audioThread, &QThread::finished, audioThread, &QObject::deleteLater);
    //stop worker thread when it necessary
    connect(this, &MyServer::stopAllWorkers, audioWorker, &AbstractWorker::stop);
    //move socket to video worker thread
    connect(this, &MyServer::audioSocketConnected, audioWorker, &AbstractWorker::getSocket);
}

bool MyServer::readSize(QTcpSocket *socket, SocketMetaData *metaData)
{
    bool result = false;
    if (socket->bytesAvailable() >= metaData->bytesForRead) {
        QByteArray data(metaData->bytesForRead, '\0');
        number_type bytesRead = 0;
        while (bytesRead < metaData->bytesForRead) {
            bytesRead += socket->read(data.data() + bytesRead, metaData->bytesForRead - bytesRead);
        }
        QDataStream desizeSerializer(data);
        desizeSerializer.setVersion(QDataStream::Qt_5_1);
        desizeSerializer.setByteOrder(QDataStream::LittleEndian);
        desizeSerializer >> metaData->bytesForRead;
        metaData->isSize = false;
        result = true;
    }
    return result;
}

bool MyServer::readData(QTcpSocket *socket, SocketMetaData *metaData, QString &resultStr)
{
    bool result = false;
    if (socket->bytesAvailable() >= metaData->bytesForRead) {
        QByteArray data(metaData->bytesForRead, '\0');
        number_type bytesRead = 0;
        while (bytesRead < metaData->bytesForRead) {
            bytesRead += socket->read(data.data() + bytesRead, metaData->bytesForRead - bytesRead);
        }
        QDataStream desizeSerializer(data);
        desizeSerializer.setVersion(QDataStream::Qt_5_1);
        desizeSerializer.setByteOrder(QDataStream::LittleEndian);
        desizeSerializer >> resultStr;
        result = true;
    }
    return result;
}

void MyServer::prepareSocket(const QString &type, int i)
{
    auto socket = tmpList.takeAt(i);
    socket->disconnect();
    if (type == "video") {
        //move socket to the video worker thread
        socket->setParent(nullptr);
        socket->moveToThread(videoThread);
        //pull socket to video thread
        emit videoSocketConnected(socket);
        qDebug() << "video socket";
    } else if (type == "audio") {
        //move socket to the video worker thread
        socket->setParent(nullptr);
        socket->moveToThread(audioThread);
        //pull socket to audio thread
        emit audioSocketConnected(socket);
        qDebug() << "audio socket";
    } else {
        //if socket not video and not audio socket delete it
        tmpList.removeAt(i);
        socket->disconnectFromHost();
        qDebug() << "undefined socket " << type;
    }
    delete tmpMetaData.takeAt(i);
}
