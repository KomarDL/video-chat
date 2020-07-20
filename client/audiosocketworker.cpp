#include "audiosocketworker.h"

AudioSocketWorker::AudioSocketWorker(QTcpSocket *socket, QObject *parent) :
    AbstractSocketWorker(socket, "audio", parent)
{

}
