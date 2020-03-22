#include "mainwindow.h"
#include "thread.h"

#include <QtWidgets>

class MainWindowPrivate{
public:
    MainWindowPrivate(QWidget *owner)
        :owner(owner){
        ipEdit = new QLineEdit(owner);
        ipEdit->setText("127.0.0.1");
        QRegExp regExp("^((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)$");
        QValidator *validator = new QRegExpValidator(regExp, ipEdit);
        ipEdit->setValidator(validator);
        portSpinBox = new QSpinBox(owner);
        portSpinBox->setRange(1, 65536);
        portSpinBox->setValue(65533);
        numSpinBox = new QSpinBox(owner);
        numSpinBox->setRange(1, 2147483647);
        numSpinBox->setValue(1);
        timeSpinBox = new QSpinBox(owner);
        timeSpinBox->setSuffix(" ms");
        timeSpinBox->setRange(1, 1000000);
        timeSpinBox->setSingleStep(50);
        timeSpinBox->setValue(1000);
        connectBtn = new QPushButton(QObject::tr("Connect"), owner);
        connectBtn->setCheckable(true);
        messageEdit = new QTextEdit(owner);
        messageEdit->document()->setMaximumBlockCount(1000);
        sendTime = new QTimer(owner);
    }
    QWidget *owner;
    QLineEdit *ipEdit;
    QSpinBox *portSpinBox;
    QSpinBox *numSpinBox;
    QSpinBox *timeSpinBox;
    QPushButton *connectBtn;
    QTextEdit *messageEdit;

    QList<Thread*> clientList;
    QTimer *sendTime;
};


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , d(new MainWindowPrivate(this))
{
    setupUI();
    buildConnect();
    qDebug() << "MainWindows: " << QThread::currentThreadId();
}

MainWindow::~MainWindow()
{
    delete d;
}

void MainWindow::onConnect()
{
    if(d->connectBtn->isChecked()){
        d->connectBtn->setChecked(false);
        QString ip = d->ipEdit->text();
        if(ip.isEmpty()){
            warningBox(tr("Please fill in the correct IP address!"), d->ipEdit);
            return;
        }
        int port = d->portSpinBox->value();
        if(port <= 0 || port > 65536){
            warningBox(tr("Please fill in the correct port address!"), d->portSpinBox);
            return;
        }
        int num = d->numSpinBox->value();
        if(num <= 0){
            warningBox(tr("Please fill in the correct number of clients (sub threads)!"), d->numSpinBox);
            return;
        }
        int time = d->timeSpinBox->value();
        if(time <= 0){
            warningBox(tr("Please enter the correct timeout!"), d->numSpinBox);
            return;
        }
        d->sendTime->setInterval(time);
        qDeleteAll(d->clientList);
        d->clientList.clear();
        for(int i=0; i<num; i++){
            Thread *thread = new Thread(ip, port, this);
            d->clientList.append(thread);
            thread->start();
            QString str = QString::number(i+1) + tr(" Clients (threads) start running");
            d->messageEdit->append(str);
        }
        //
        // fix me?
        // Keep the window, clear the thread list, stop the timer,
        // and the "connectBtn" button is displayed as connect.
        // In the connected state, closing the window will not crash.
        //
        //connect(d->clientList[0], &Thread::destroyed, this, &MainWindow::onStop);
        connect(d->clientList[0], &Thread::destroyed, qApp, &QApplication::quit);
        d->sendTime->start();
        changeControlState(false);
        d->connectBtn->setText(tr("Connected"));
        d->connectBtn->setChecked(true);
    }else{
        d->sendTime->stop();
        foreach(Thread* thread, d->clientList){
            if(thread && thread->isRunning()){
                thread->quit();
                thread->wait();
            }
        }
        d->clientList.clear();
        changeControlState(true);
        d->connectBtn->setText(tr("Connect"));
        d->connectBtn->setChecked(false);
    }
}

void MainWindow::onWrite()
{
    QString str = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QByteArray buf = str.toLatin1() + " Hello World!";
    foreach(Thread* thread, d->clientList){
        if(thread && thread->isRunning()){
            emit thread->writeToServer(buf);
        }
    }
}

//void MainWindow::onStop()
//{
//    d->sendTime->stop();
//    d->clientList.clear();
//    changeControlState(true);
//    d->connectBtn->setText(tr("Connect"));
//    d->connectBtn->setChecked(false);
//}

void MainWindow::warningBox(const QString &str, QWidget *w)
{
    QMessageBox::warning(this, tr("Warining"), str, QMessageBox::Ok);
    w->setFocus();
}

void MainWindow::buildConnect()
{
    connect(d->connectBtn, &QPushButton::clicked, this, &MainWindow::onConnect);
    connect(d->sendTime, &QTimer::timeout, this, &MainWindow::onWrite);
}

void MainWindow::setupUI()
{
    QGridLayout *gLayout = new QGridLayout;
    gLayout->addWidget(new QLabel(tr("Sever IP: "), this), 0, 0);
    gLayout->addWidget(d->ipEdit, 0, 1);
    gLayout->addWidget(new QLabel(tr("Server Port: "), this), 0, 2);
    gLayout->addWidget(d->portSpinBox, 0, 3);
    gLayout->addWidget(new QLabel(tr("Client(Threaths) Numbers: "), this), 1, 0);
    gLayout->addWidget(d->numSpinBox, 1, 1);
    gLayout->addWidget(new QLabel(tr("Timeout: "), this), 1, 2);
    gLayout->addWidget(d->timeSpinBox, 1, 3);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addLayout(gLayout);
    btnLayout->addWidget(d->connectBtn);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(btnLayout);
    layout->addWidget(d->messageEdit);

    QFrame *frame = new QFrame(this);
    frame->setLayout(layout);

    setCentralWidget(frame);
}

void MainWindow::changeControlState(bool state)
{
    d->ipEdit->setEnabled(state);
    d->portSpinBox->setEnabled(state);
    d->numSpinBox->setEnabled(state);
    d->timeSpinBox->setEnabled(state);
}

