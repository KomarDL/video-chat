#ifndef AUDIOSOCKETWORKER_H
#define AUDIOSOCKETWORKER_H

#include <QObject>
#include <QTcpSocket>
#include "config.h"
#include "abstractsocketworker.h"

class AudioSocketWorker : public AbstractSocketWorker
{
    Q_OBJECT
public:
    explicit AudioSocketWorker(QTcpSocket* socket, QObject *parent = nullptr);
};

#endif // AUDIOSOCKETWORKER_H
