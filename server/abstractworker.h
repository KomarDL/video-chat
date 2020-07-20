#ifndef ABSTRACTWORKER_H
#define ABSTRACTWORKER_H

#include <QObject>
#include <QTcpSocket>
#include "socketmetadata.h"

class AbstractWorker : public QObject
{
    Q_OBJECT
public:
    explicit AbstractWorker(QObject *parent = nullptr);
    virtual ~AbstractWorker();

signals:
    void finished();

public slots:
    virtual void process();
    void stop();
    void getSocket(QTcpSocket* socket);


protected slots:
    virtual void read();
    virtual void handleError(QAbstractSocket::SocketError err);

protected:
    MetaDataList metaData;
    SocketList socketList;

    bool isStop = false;
    bool isDataRead = false;

    bool readSize(QTcpSocket* socket, SocketMetaData *meta);
    QByteArray readData(QTcpSocket* socket, SocketMetaData *meta);

    template<typename T>
    QByteArray serialize(T data);
    template<typename T>
    void sendData(const T& data, const int i);

private:
    void send(const QByteArray& data, const int i);
};

template<typename T>
QByteArray AbstractWorker::serialize(T data)
{
    QByteArray result;
    QDataStream dataSerializer(&result, QIODevice::WriteOnly);
    dataSerializer.setVersion(QDataStream::Qt_5_1);
    dataSerializer.setByteOrder(QDataStream::LittleEndian);
    dataSerializer << data;
    return result;
}

template<typename T>
void AbstractWorker::sendData(const T &data, const int i)
{
    QByteArray serializedData = serialize(data);
    QByteArray serializedSize = serialize( static_cast<number_type>(serializedData.size()) );
    send(serializedSize, i);
    send(serializedData, i);
}

#endif // ABSTRACTWORKER_H
