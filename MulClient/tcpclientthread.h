#ifndef THREAD_H
#define THREAD_H

#include <QThread>

class TcpClientThreadPrivate;
class TcpClientThread : public QThread
{
    Q_OBJECT
public:
    TcpClientThread(const QString &ip,
                    const quint16 port,
                    const int index,
                    QObject *parent = nullptr);
    ~TcpClientThread() override;

signals:
    void writeToServer(const QByteArray&);
    void quitThread(const int);

private slots:
    void removeMyself();

protected:
    void run() override;

private:
    QScopedPointer<TcpClientThreadPrivate> d;
};

#endif // THREAD_H
