#ifndef VIDEOSOCKETWORKER_H
#define VIDEOSOCKETWORKER_H

#include <QObject>
#include <QTcpSocket>
#include <QImage>
#include "config.h"
#include "abstractsocketworker.h"

class VideoSocketWorker : public AbstractSocketWorker
{
    Q_OBJECT

public:
    explicit VideoSocketWorker(QTcpSocket* socket, QObject *parent = nullptr);

public slots:
    void send(QImage img);

};

#endif // VIDEOSOCKETWORKER_H
