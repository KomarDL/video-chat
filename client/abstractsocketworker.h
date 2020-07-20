#ifndef ABSTRACTSOCKETWORKER_H
#define ABSTRACTSOCKETWORKER_H

#include <QObject>
#include <QTcpSocket>
#include "config.h"

class AbstractSocketWorker : public QObject
{
    Q_OBJECT
public:
    explicit AbstractSocketWorker(QTcpSocket *socket, QString socketType, QObject *parent = nullptr);
    virtual ~AbstractSocketWorker();
signals:
    void dataReady(QByteArray);
    void errorOccured(QString);
    void finished();
public slots:
    virtual void process();
    virtual void stop();
    virtual void send(QByteArray data);
protected slots:
    virtual void socketErrorOccured(QAbstractSocket::SocketError err);
    virtual void read();

protected:
    QTcpSocket *socket;
    number_type dataSize = sizeof(number_type);

    bool isStop = false;
    bool isSize = true;
    bool isDataRead = false;

    template<typename T>
    QByteArray serialize(T data);
    template<typename T>
    void sendData(T data);

    bool readSize();
    QByteArray readData();

private:
    void sendPart(const QByteArray& data);
    QString socketType;
};

template<typename T>
QByteArray AbstractSocketWorker::serialize(T data)
{
    QByteArray result;
    QDataStream dataSerializer(&result, QIODevice::WriteOnly);
    dataSerializer.setVersion(QDataStream::Qt_5_1);
    dataSerializer.setByteOrder(QDataStream::LittleEndian);
    dataSerializer << data;
    return result;
}

template<typename T>
void AbstractSocketWorker::sendData(T data)
{
    QByteArray serializedData = serialize(data);
    QByteArray serializedSize = serialize( static_cast<number_type>(serializedData.size()) );
    //send data size
    sendPart(serializedSize);
    //send data
    sendPart(serializedData);
}



#endif // ABSTRACTSOCKETWORKER_H
