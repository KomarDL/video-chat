#include "cameraview.h"
#include <QPainter>
#include <QDebug>

CameraView::CameraView(QWidget *parent)
    : QFrame(parent)
    , image(new QImage)
    , timer(new QTimer)
{
    connect(timer.get(), &QTimer::timeout, this, &CameraView::clearView);
    timer->setSingleShot(true);
}

void CameraView::setImage(QImage *newImage)
{
    image.reset(newImage);
    timer->start(5 * 1000);
    update();
}

void CameraView::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    if (!image->isNull()) {
        QImage resImg = image->scaled(this->size(), Qt::KeepAspectRatio);
        auto pos = leftTopCornerPos(resImg);
        p.drawImage(pos, resImg);
    } else {
        p.fillRect(this->rect(), p.brush());
    }
}

void CameraView::clearView()
{
    this->setImage(new QImage);
}

QPoint CameraView::leftTopCornerPos(const QImage &img)
{
    return QPoint ((this->width() - img.width()) / 2,
                  (this->height() - img.height()) / 2);
}
