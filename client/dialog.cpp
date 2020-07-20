#include "dialog.h"
#include "ui_dialog.h"
#include <QMessageBox>
#include <QTimer>

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    this->setWindowTitle("Video client");
    this->setFixedSize(this->size());
    setIPValidator();
}

Dialog::~Dialog()
{
    delete ui;
}


void Dialog::on_connectButton_clicked()
{
    auto ipStr = ui->ipLineEdit->text();
    auto pos = 0;
    if (ui->ipLineEdit->validator()->validate(ipStr, pos) == QValidator::Acceptable) {
        initSockets(ipStr, ui->portSpinBox->value());
        if (!isVideoError && !isAudioError) {
            ui->infoLabel->setText("Connecting...");
            ui->connectButton->setEnabled(false);
            QTimer::singleShot(5 * 1000, this, SLOT(connectionFailed()));
        }
    } else {
        QMessageBox::information(this, "Ip error", "Incorrect ip address");
    }

}

void Dialog::videoSocketConnected()
{
    isVideoConnected = true;
    isAudioConnected = true;
    checkAndOpenMainWindow();
}

void Dialog::videoSocketErrorOccured(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)
    isVideoError = true;
    if (!isAudioError) {
        QTimer::singleShot(0, this, SLOT(closeSockets()));
        QMessageBox::information(this, "Something went wrong", videoSocket->errorString());
        enableButton();
    }
}

void Dialog::audioSocketConnected()
{
    isVideoConnected = true;
}

void Dialog::audioSocketErrorOccured(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)
    isAudioError = true;
    if (!isVideoError) {
        QTimer::singleShot(0, this, SLOT(closeSockets()));
        QMessageBox::information(this, "Something went wrong", audioSocket->errorString());
        enableButton();
    }
}

void Dialog::connectionFailed()
{
    //if sockets didn't connect
    if (!isVideoConnected || !isAudioConnected) {
        // if no errors occured
        if (!isVideoError && !isAudioError) {
            closeSockets();
            QMessageBox::information(this, "Connection error", "Unable connect to the server");
        }
        enableButton();
    }
}

void Dialog::closeSockets()
{
    videoSocket->close();
    audioSocket->close();
}

void Dialog::enableButton()
{
    ui->infoLabel->clear();
    ui->connectButton->setEnabled(true);
}


void Dialog::setIPValidator()
{
    /* create regex for octet */
    QString ipRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";

    // create regex for ip
    QRegExp ipRegex ("^" + ipRange
                     + "\\." + ipRange
                     + "\\." + ipRange
                     + "\\." + ipRange + "$");
    // create regex validator
    auto ipValidator = new QRegExpValidator(ipRegex, this);
    /* set validator for QLineEdit */
    ui->ipLineEdit->setValidator(ipValidator);
}

void Dialog::createSockets()
{
    if (videoSocket != nullptr) {
        delete videoSocket;
    }
    if (audioSocket != nullptr) {
        delete audioSocket;
    }
    videoSocket = new QTcpSocket;
    audioSocket = new QTcpSocket;

}

void Dialog::initSockets(QString hostAddress, quint16 port)
{
    //recreate sockets
    createSockets();

    connect(videoSocket, SIGNAL(connected()), this, SLOT(videoSocketConnected()));
    connect(audioSocket, SIGNAL(connected()), this, SLOT(audioSocketConnected()));
    connect(videoSocket, &QAbstractSocket::errorOccurred, this, &Dialog::videoSocketErrorOccured);
    connect(audioSocket, &QAbstractSocket::errorOccurred, this, &Dialog::audioSocketErrorOccured);

    initFlags();
    // connect to the server
    videoSocket->connectToHost(hostAddress, port);
    audioSocket->connectToHost(hostAddress, port);
}

void Dialog::initFlags()
{
    isVideoError = isVideoConnected = false;
    isAudioError = isAudioConnected = false;
}

void Dialog::checkAndOpenMainWindow()
{
    if (isSocketsConnected()) {
        setHidden(true);
        auto mainWindow = new MainWindow(videoSocket, audioSocket, this);
        mainWindow->show();
    }
}

