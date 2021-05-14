#include "logasync.h"
#include "fileutil.h"

#include <QDateTime>
#include <QWaitCondition>

// 消息处理函数
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if(type < LogAsync::instance()->logLevel())
        return;

    QString level;
    switch(type){
    case QtDebugMsg: level = QLatin1String("Debug"); break;
    case QtWarningMsg: level = QLatin1String("Warning"); break;
    case QtCriticalMsg: level = QLatin1String("Critica"); break;
    case QtFatalMsg: level = QLatin1String("Fatal"); break;
    case QtInfoMsg: level = QLatin1String("Info"); break;
    default: level = QLatin1String("Unknown"); break;
    }

    const QString dataTimeString(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));
    const QString threadId(QString::number(qulonglong(QThread::currentThreadId())));
    QString contexInfo;
#ifndef QT_NO_DEBUG
    contexInfo = QString("File:(%1) Line:(%2)").arg(context.file).arg(context.line);
#endif
    const QString message = QString("%1 %2 [%3] %4 - %5\n")
                                .arg(dataTimeString, threadId, level, msg, contexInfo);

    switch (LogAsync::instance()->orientation()) {
    case LogAsync::Orientation::Std:
        fprintf(stderr, "%s", message.toLocal8Bit().constData());
        break;
    case LogAsync::Orientation::File:
        emit LogAsync::instance()->appendBuf(message);
        break;
    case LogAsync::Orientation::StdAndFile:
        fprintf(stderr, "%s", message.toLocal8Bit().constData());
        emit LogAsync::instance()->appendBuf(message);
        break;
    default:
        fprintf(stderr, "%s", message.toLocal8Bit().constData());
        break;
    }
}

struct LogAsyncPrivate{
    QtMsgType msgType = QtWarningMsg;
    LogAsync::Orientation orientation = LogAsync::Orientation::Std;
    QWaitCondition waitCondition;
    QMutex mutex;
};

QMutex LogAsync::m_mutex;

LogAsync *LogAsync::instance()
{
    QMutexLocker locker(&m_mutex);
    static LogAsync log;
    return &log;
}


void LogAsync::setOrientation(LogAsync::Orientation orientation)
{
    d_ptr->orientation = orientation;
}

LogAsync::Orientation LogAsync::orientation()
{
    return d_ptr->orientation;
}

void LogAsync::setLogLevel(QtMsgType type)
{
    d_ptr->msgType = type;
}

QtMsgType LogAsync::logLevel()
{
    return d_ptr->msgType;
}

void LogAsync::startWork()
{
    start();
    QMutexLocker lock(&d_ptr->mutex);
    d_ptr->waitCondition.wait(&d_ptr->mutex);
}

void LogAsync::stop()
{
    if(isRunning()){
        //QThread::sleep(1);   // 最后一条日志格式化可能来不及进入信号槽
        quit();
        wait();
    }
}

void LogAsync::run()
{
    FileUtil fileUtil;
    connect(this, &LogAsync::appendBuf, &fileUtil, &FileUtil::onWrite);
    d_ptr->waitCondition.wakeOne();
    exec();
}

LogAsync::LogAsync(QObject *parent) : QThread(parent)
    , d_ptr(new LogAsyncPrivate)
{
    qInstallMessageHandler(messageHandler);
}

LogAsync::~LogAsync()
{
    stop();
    qInstallMessageHandler(nullptr);
    fprintf(stderr, "%s\n", "~LogAsync");
}
