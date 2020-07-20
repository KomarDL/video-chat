#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <memory>
#include "myserver.h"
#include "videoworker.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT
    using server_ptr = std::unique_ptr<MyServer>;
public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

private:
    Ui::Dialog *ui;
    server_ptr server;

    void initServer();

    friend class VideoWorker;

};
#endif // DIALOG_H
