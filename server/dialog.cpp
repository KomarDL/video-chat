#include "dialog.h"
#include "ui_dialog.h"
#include <QMessageBox>
#include <QtNetwork>

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
    , server(new MyServer(this))
{
    ui->setupUi(this);
    this->setFixedSize(this->size());
    initServer();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::initServer()
{

    if (!server->listen()) {
        QMessageBox::critical(this, tr("Fortune Server"),
                              tr("Unable to start the server: %1.")
                              .arg(server->errorString()));
        close();
        return;
    }
    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
                ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    ui->ipLabel->setText(ipAddress);
    ui->portLabel->setNum(server->serverPort());
}

