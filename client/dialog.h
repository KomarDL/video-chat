#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTcpSocket>
#include <QRegExpValidator>
#include "mainwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

private slots:
    void on_connectButton_clicked();

    void videoSocketConnected();
    void videoSocketErrorOccured(QAbstractSocket::SocketError socketError);

    void audioSocketConnected();
    void audioSocketErrorOccured(QAbstractSocket::SocketError socketError);

    void connectionFailed();
    void closeSockets();

    void enableButton();

private:
    Ui::Dialog *ui;
    QTcpSocket* videoSocket{};
    QTcpSocket* audioSocket{};
    bool isVideoError = false;
    bool isAudioError = false;
    bool isVideoConnected = false;
    bool isAudioConnected = false;


    void setIPValidator();
    void createSockets();
    void connectToSocketSignals(QTcpSocket* socket);
    void connectSocketsToHost(QString hostAddress, quint16 port);
    void initSockets(QString hostAddress, quint16 port);
    void initFlags();
    void checkAndOpenMainWindow();
    inline bool isSocketsConnected() { return isVideoConnected && isAudioConnected; }
};
#endif // DIALOG_H
