#ifndef AUDIOWORKER_H
#define AUDIOWORKER_H

#include <QObject>
#include <QTcpSocket>
#include "abstractworker.h"

class AudioWorker : public AbstractWorker
{
    Q_OBJECT
public:
    explicit AudioWorker(QObject *parent = nullptr);

};

#endif // AUDIOWORKER_H
