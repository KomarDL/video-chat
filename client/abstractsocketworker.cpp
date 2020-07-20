#include "abstractsocketworker.h"
#include <QApplication>

AbstractSocketWorker::AbstractSocketWorker(QTcpSocket *socket, QString socketType, QObject *parent) :
    QObject(parent),
    socket(socket),
    socketType(socketType)
{
    //set the worker as the parent of the socket
    //so that the socket automatically moves to the worker thread
    socket->setParent(this);
}

AbstractSocketWorker::~AbstractSocketWorker()
{

}

void AbstractSocketWorker::process()
{
    connect(socket, &QAbstractSocket::errorOccurred, this, &AbstractSocketWorker::socketErrorOccured);
    connect(socket, &QIODevice::readyRead, this, &AbstractSocketWorker::read);
    sendData(socketType);
    while (!isStop) {
        qApp->processEvents();
    }
}

void AbstractSocketWorker::stop()
{
    isStop = true;
}

void AbstractSocketWorker::send(QByteArray data)
{
    sendData(data);
}

void AbstractSocketWorker::socketErrorOccured(QAbstractSocket::SocketError err)
{
    Q_UNUSED(err)
    isStop = true;
    socket->disconnectFromHost();
    socket->waitForDisconnected();
    emit errorOccured(socket->errorString());
    emit finished();
}

void AbstractSocketWorker::read()
{
    if (isSize) {
        if (!readSize()) {
            return;
        }
        isSize = false;
    }
    auto data = readData();

    if (isDataRead) {
        emit dataReady(data);
        isSize = true;
    }
}

bool AbstractSocketWorker::readSize()
{
    bool result = false;
    auto tmp = socket->bytesAvailable();
    dataSize = sizeof (number_type);
    if (tmp >= dataSize) {
        QByteArray data(dataSize, '\0');
        number_type bytesRead = 0;
        while (bytesRead < dataSize) {
            bytesRead += socket->read(data.data() + bytesRead, dataSize - bytesRead);
        }
        QDataStream desizeSerializer(data);
        desizeSerializer.setVersion(QDataStream::Qt_5_1);
        desizeSerializer.setByteOrder(QDataStream::LittleEndian);
        desizeSerializer >> dataSize;
        result = true;
    }
    return result;
}

QByteArray AbstractSocketWorker::readData()
{
    QByteArray result;
    isDataRead = false;
    auto tmp = socket->bytesAvailable();
    if (tmp >= dataSize) {
        QByteArray data(dataSize, '\0');
        number_type bytesRead = 0;
        while (bytesRead < dataSize) {
            bytesRead += socket->read(data.data() + bytesRead, dataSize - bytesRead);
        }
        result = data;
        dataSize = sizeof (number_type);
        isDataRead = true;
    }
    return result;
}

void AbstractSocketWorker::sendPart(const QByteArray &data)
{
    number_type bytesWrited = 0;
    while (bytesWrited < static_cast<number_type>(data.size())) {
        auto writed = socket->write(data.data() + bytesWrited, data.size() - bytesWrited);
        if (writed != -1){
            bytesWrited += writed;
        } else {
            qDebug() << socket->errorString();
            break;
        }
    }
}
