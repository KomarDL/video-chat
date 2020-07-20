#include "videosocketworker.h"
#include <QApplication>
#include <QImageWriter>
#include <QBuffer>

VideoSocketWorker::VideoSocketWorker(QTcpSocket *socket, QObject *parent) :
    AbstractSocketWorker(socket, "video", parent)
{
}

void VideoSocketWorker::send(QImage img)
{
    QBuffer buff;
    buff.open(QIODevice::ReadWrite);
    img.save(&buff, "JPG");
    sendData(buff.buffer());
}
