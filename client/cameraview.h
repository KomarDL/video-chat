#ifndef Canvas_H
#define Canvas_H

#include <QFrame>
#include <QImage>
#include <memory>
#include <QPoint>
#include <QTimer>

class CameraView : public QFrame
{
    using image_ptr = std::unique_ptr<QImage>;
    using timer_ptr = std::unique_ptr<QTimer>;

    Q_OBJECT
public:
    CameraView(QWidget *parent = nullptr);

    void setImage(QImage *newImage);

protected:
    void paintEvent(QPaintEvent *) override;
private slots:
    void clearView();
private:
    image_ptr image;
    timer_ptr timer;

    QPoint leftTopCornerPos(const QImage& img);

};

#endif // Canvas_H
