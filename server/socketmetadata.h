#ifndef SOCKETMETADATA_H
#define SOCKETMETADATA_H

#include <QTcpSocket>
#include "../client/config.h"

struct SocketMetaData {
    SocketMetaData(number_type expectedBytesAmount = sizeof(number_type), bool isSize = true) : bytesForRead(expectedBytesAmount), isSize(isSize) {}
    number_type bytesForRead;
    bool isSize;
};

using MetaDataList = QVector<SocketMetaData*>;
using SocketList = QVector<QTcpSocket*>;

#endif // SOCKETMETADATA_H
