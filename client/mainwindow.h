#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <memory>
#include <QAudioInput>
#include <QAudioOutput>
#include <QAudioFormat>
#include <QIODevice>
#include <QAudioDecoder>
#include <opencv2/videoio.hpp>
#include "config.h"
#include "cameraview.h"
#include "videosocketworker.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    using audioinput_prt = std::unique_ptr<QAudioInput>;
    using audiooutput_prt = std::unique_ptr<QAudioOutput>;
    using audiodecoder_ptr = std::unique_ptr<QAudioDecoder>;
public:
    explicit MainWindow(QTcpSocket* videoSocket, QTcpSocket* audioSocket, QWidget *parent = nullptr);
    ~MainWindow();
signals:
    void stopAllWorkers();
    void frameReady(QImage);
    void sampleReady(QByteArray);

public slots:
    void displayFrame(QByteArray frameData);
    void workerError(QString reason);
    void captureVideo();
    void playSample(QByteArray sample);
private slots:
    void prepareSample();
protected:
    void closeEvent(QCloseEvent *event);
private:
    Ui::MainWindow *ui;
    CameraView *mainCameraView;
    CameraView *clientCameraView;
    QTcpSocket *videoSocket;
    QTcpSocket *audioSocket;
    QThread *videoThread;
    QThread *audioThread;
    cv::VideoCapture cap;

    bool isStop = false;

    QAudioInput *audioInput = nullptr;
    QAudioOutput *audioOutput = nullptr;
    QAudioDecoder *audioDecoder = nullptr;
    QIODevice* outDevice;
    QIODevice* inDevice;

    void configurVideo();
    void configurAudio();
    FramePair deserialize(QByteArray& frameData);
};

#endif // MAINWINDOW_H
