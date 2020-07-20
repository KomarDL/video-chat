#ifndef VIDEOWORKER_H
#define VIDEOWORKER_H

#include <QObject>
#include <QTcpSocket>
#include "abstractworker.h"

class VideoWorker : public AbstractWorker
{
    Q_OBJECT

public:
    explicit VideoWorker(QObject *parent = nullptr);

private slots:
    void read() override;
    void handleError(QAbstractSocket::SocketError err) override;
};

#endif // VIDEOWORKER_H
