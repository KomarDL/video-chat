#ifndef CONFIG_H
#define CONFIG_H

#include <QMetaType>
#include <QPair>
#include <QPixmap>

using number_type = quint32;
using FramePair = QPair<number_type, QByteArray>;
Q_DECLARE_METATYPE(FramePair)

#endif // CONFIG_H
