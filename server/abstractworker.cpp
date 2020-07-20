#include "abstractworker.h"
#include <QApplication>

AbstractWorker::AbstractWorker(QObject *parent) : QObject(parent)
{

}

AbstractWorker::~AbstractWorker()
{
    for (auto item : metaData) {
        delete item;
    }
}

void AbstractWorker::process()
{
    while (!isStop) {
        qApp->processEvents();
    }
    emit finished();
}

void AbstractWorker::stop()
{
    isStop = true;
}

void AbstractWorker::getSocket(QTcpSocket *socket)
{
    socket->setParent(this);
    connect(socket, &QIODevice::readyRead, this, &AbstractWorker::read);
    connect(socket, &QAbstractSocket::errorOccurred, this, &AbstractWorker::handleError);
    socketList.push_back(socket);
    metaData.push_back(new SocketMetaData);
}

void AbstractWorker::read()
{
    auto senderSocket = dynamic_cast<QTcpSocket*>(sender());
    auto i = socketList.indexOf(senderSocket);
    if (i != -1) {
        if (metaData[i]->isSize) {
            if (!readSize(senderSocket, metaData[i])) {
                return;
            }
            metaData[i]->isSize = false;
        }
        auto data = readData(senderSocket, metaData[i]);
        if (isDataRead) {
            sendData(data, i);
        }
    } else {
        //this message must never appear
        qDebug() << "socket not in socket list";
    }

}

void AbstractWorker::handleError(QAbstractSocket::SocketError err)
{
    Q_UNUSED(err)
    auto senderSocket = dynamic_cast<QTcpSocket*>(sender());
    qInfo() << senderSocket->errorString();
    senderSocket->disconnectFromHost();
    auto i = socketList.indexOf(senderSocket);
    socketList.removeAt(i);
    delete metaData.takeAt(i);
}

bool AbstractWorker::readSize(QTcpSocket *socket, SocketMetaData *meta)
{
    bool result = false;
    auto tmp = socket->bytesAvailable();
    auto needBytes = sizeof (number_type);
    if (static_cast<number_type>(tmp) >= needBytes) {
        QByteArray data(needBytes, '\0');
        number_type bytesRead = 0;
        while (bytesRead < needBytes) {
            bytesRead += socket->read(data.data() + bytesRead, needBytes - bytesRead);
        }
        QDataStream desizeSerializer(data);
        desizeSerializer.setVersion(QDataStream::Qt_5_1);
        desizeSerializer.setByteOrder(QDataStream::LittleEndian);
        desizeSerializer >> meta->bytesForRead;
        meta->isSize = false;
        result = true;
    }

    return result;
}

QByteArray AbstractWorker::readData(QTcpSocket *socket, SocketMetaData *meta)
{
    isDataRead = false;
    QByteArray result;
    auto tmp = socket->bytesAvailable();
    if (tmp >= meta->bytesForRead) {
        QByteArray data(meta->bytesForRead, '\0');
        number_type bytesRead = 0;
        while (bytesRead < meta->bytesForRead) {
            bytesRead += socket->read(data.data() + bytesRead, meta->bytesForRead - bytesRead);
        }

        QDataStream desizeSerializer(data);
        desizeSerializer.setVersion(QDataStream::Qt_5_1);
        desizeSerializer.setByteOrder(QDataStream::LittleEndian);
        desizeSerializer >> result;

        meta->isSize = true;
        meta->bytesForRead = sizeof (number_type);
        isDataRead = true;
    }
    return result;
}

void AbstractWorker::send(const QByteArray &data, const int i)
{
    for (auto j = 0; j < socketList.length(); ++j) {
        if (j != i) {
            number_type bytesWrited = 0;
            auto socket = socketList[j];
            while (bytesWrited < static_cast<number_type>(data.size())) {
                auto writed = socket->write(data.data() + bytesWrited, data.size() - bytesWrited);
                if (writed != -1) {
                    bytesWrited += writed;
                } else {
                    qDebug() << socket->errorString();
                    break;
                }
            }
        }
    }
}
