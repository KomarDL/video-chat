#include "videoworker.h"
#include <QApplication>
#include <QTimer>

VideoWorker::VideoWorker(QObject *parent) : AbstractWorker(parent)
{
}

void VideoWorker::read()
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
            FramePair pair(i, data);
            sendData(pair, i);
        }
    }
}

void VideoWorker::handleError(QAbstractSocket::SocketError err)
{
    Q_UNUSED(err)
    auto senderSocket = dynamic_cast<QTcpSocket*>(sender());
    qInfo() << senderSocket->errorString();
    senderSocket->disconnectFromHost();

    auto i = socketList.indexOf(senderSocket);

    FramePair emptyFrame;
    emptyFrame.first = i;
    sendData(emptyFrame, i);

    socketList.removeAt(i);
    delete metaData.takeAt(i);
}
