#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QTimer>
#include <QThread>
#include <QMessageBox>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "audiosocketworker.h"

MainWindow::MainWindow(QTcpSocket* videoSocket, QTcpSocket* audioSocket, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mainCameraView(new CameraView(this)),
    clientCameraView(new CameraView(this)),
    videoSocket(videoSocket),
    audioSocket(audioSocket),
    videoThread(new QThread),
    audioThread(new QThread)
{
    ui->setupUi(this);
    ui->gridLayout->addWidget(mainCameraView);
    ui->gridLayout->addWidget(clientCameraView, 0, 1);

    configurVideo();
    configurAudio();
    videoThread->start();
    audioThread->start();
    QTimer::singleShot(0, this, SLOT(captureVideo()));

    qRegisterMetaType<FramePair>();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::displayFrame(QByteArray frameData)
{
    FramePair framePair = deserialize(frameData);
    if (!framePair.second.isEmpty()){
        auto img = QImage::fromData(framePair.second, "JPG");
        clientCameraView->setImage(new QImage(img));
    } else {
        clientCameraView->setImage(new QImage);
    }
}

void MainWindow::workerError(QString reason)
{
    emit stopAllWorkers();
    audioInput->stop();
    audioOutput->stop();
    QMessageBox::information(this, "Error info", reason);
    isStop = true;
    videoThread->terminate();
    audioThread->terminate();
    videoThread->wait();
    audioThread->wait();

    this->close();
}

void MainWindow::captureVideo()
{
    cv::Mat frame;
    //--- INITIALIZE VIDEOCAPTURE
    // open the default camera using default API
    // cap.open(0);
    // OR advance usage: select any API backend
    int deviceID = 0;             // 0 = open default camera
    int apiID = cv::CAP_ANY;      // 0 = autodetect default API
    // open selected camera using selected API
    cap.open(deviceID, apiID);

    //--- GRAB AND WRITE LOOP
    while (cap.isOpened() && !isStop)
    {
        // wait for a new frame from camera and store it into 'frame'
        cap.read(frame);
        // check if we succeeded
        if (frame.empty()) {
            qDebug() << "ERROR! blank frame grabbed\n";
            break;
        }

        // show live
        cv::resize(frame, frame, cv::Size(512, 512));
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        auto image = new QImage(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
        //send video
        emit frameReady(*image);
        //display video
        mainCameraView->setImage(image);
        qApp->processEvents();
    }
    if (!cap.isOpened()) {
        emit workerError("Unable to open camera");
    }
    cap.release();
    isStop = true;

}

void MainWindow::playSample(QByteArray sample)
{
    if (outDevice->isOpen()){
        QByteArray result;
        QDataStream desizeSerializer(sample);
        desizeSerializer.setVersion(QDataStream::Qt_5_1);
        desizeSerializer.setByteOrder(QDataStream::LittleEndian);
        desizeSerializer >> result;
        outDevice->write(result);
    }
}

void MainWindow::prepareSample()
{
    auto len = audioInput->bytesReady();
    if (len) {
        auto buffer = inDevice->read(len);
        if (buffer.length()) {
            emit sampleReady(buffer);
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    isStop = true;
}

void MainWindow::configurVideo()
{
    //create worker
    auto videoWorker = new VideoSocketWorker(videoSocket);

    videoWorker->moveToThread(videoThread);
    //start worker when thread started
    connect(videoThread, &QThread::started, videoWorker, &AbstractSocketWorker::process);
    //delete objects when thread finished work
    connect(videoWorker, &AbstractSocketWorker::finished, videoWorker, &QObject::deleteLater);
    connect(videoThread, &QThread::finished, videoThread, &QObject::deleteLater);
    //stop worker thread when it necessary
    connect(this, &MainWindow::stopAllWorkers, videoWorker, &AbstractSocketWorker::stop);
    //send frame over network when new frame available
    connect(this, &MainWindow::frameReady, videoWorker, &VideoSocketWorker::send);
    //display frame when new frame received
    connect(videoWorker, &AbstractSocketWorker::dataReady, this, &MainWindow::displayFrame);
    //send when error occured
    connect(videoWorker, &AbstractSocketWorker::errorOccured, this, &MainWindow::workerError);
}

void MainWindow::configurAudio()
{
    //create worker
    auto audioWorker = new AudioSocketWorker(audioSocket);

    audioWorker->moveToThread(audioThread);
    //start worker when thread started
    connect(audioThread, &QThread::started, audioWorker, &AbstractSocketWorker::process);
    //delete objects when thread finished work
    connect(audioWorker, &AbstractSocketWorker::finished, audioWorker, &QObject::deleteLater);
    connect(audioThread, &QThread::finished, audioThread, &QObject::deleteLater);
    //stop worker thread when it necessary
    connect(this, &MainWindow::stopAllWorkers, audioWorker, &AbstractSocketWorker::stop);
    //send frame over network when new frame available
    connect(this, &MainWindow::sampleReady, audioWorker, &AbstractSocketWorker::send);
    //display frame when new frame received
    connect(audioWorker, &AbstractSocketWorker::dataReady, this, &MainWindow::playSample);
    //send when error occured
    connect(audioWorker, &AbstractSocketWorker::errorOccured, this, &MainWindow::workerError);

    QAudioFormat format;
    // Set up the desired format, for example:
    format.setSampleRate(10000);
    format.setChannelCount(1);
    format.setSampleSize(32);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    //Init audio input
    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    if (!info.isFormatSupported(format)) {
        qWarning() << "Default format not supported, trying to use the nearest.";
        format = info.nearestFormat(format);
    }

    audioInput = new QAudioInput(format, this);

    //Init audio output
    info = QAudioDeviceInfo::defaultOutputDevice();
    if (!info.isFormatSupported(format)) {
        qWarning() << "Default format not supported, trying to use the nearest.";
        format = info.nearestFormat(format);
    }

    audioOutput = new QAudioOutput(format, this);

    //Start capture sound
    inDevice = audioInput->start();
    outDevice = audioOutput->start();

    connect(inDevice, SIGNAL(readyRead()), SLOT(prepareSample()));
}

FramePair MainWindow::deserialize(QByteArray &frameData)
{
    FramePair result;
    QDataStream dataSerializer(&frameData, QIODevice::ReadOnly);
    dataSerializer.setVersion(QDataStream::Qt_5_1);
    dataSerializer.setByteOrder(QDataStream::LittleEndian);
    dataSerializer >> result;
    return result;
}
