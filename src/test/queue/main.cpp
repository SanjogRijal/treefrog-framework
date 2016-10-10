#include <QTest>
#include <QThread>
#include <unistd.h>
#include <atomic>
#include "tqueue.h"

TQueue<quint64> intQueue;
std::atomic<quint64> generator {0};


class PopThread : public QThread
{
    Q_OBJECT
public:
    PopThread() { }
protected:
    void run() {
        quint64 lastNum = 0;
        for (;;) {
            quint64 num;
            if (intQueue.dequeue(num)) {
                QVERIFY(num == lastNum);
                lastNum++;
            }
            QThread::yieldCurrentThread();
        }
    }
};

class PushThread : public QThread
{
    Q_OBJECT
public:
    PushThread() { }
protected:
    void run() {
        for (;;) {
            if (intQueue.count() < 1000) {
                intQueue.enqueue(generator.fetch_add(1));
                //printf("%lld\n", generator.load());
            }
            QThread::yieldCurrentThread();
        }
    }
};

class TestQueue : public QObject
{
    Q_OBJECT
public slots:
    void startPopThread();
    void startPushThread();

private slots:
    void queue();
};


void TestQueue::queue()
{
    // Starts threads
    startPushThread();
    startPopThread();

    QElapsedTimer timer;
    timer.start();

    QEventLoop eventLoop;
    while (timer.elapsed() < 10000) {
        eventLoop.processEvents();
    }
    printf("queue count=%d\n", (int)intQueue.count());
    _exit(0);
}


void TestQueue::startPopThread()
{
    auto *threada = new PopThread();
    //connect(threada, SIGNAL(finished()), this, SLOT(startPopThread()));
    connect(threada, SIGNAL(finished()), threada, SLOT(deleteLater()));
    threada->start();
}


void TestQueue::startPushThread()
{
    auto *threadb = new PushThread();
    //connect(threadb, SIGNAL(finished()), this, SLOT(startPushThread()));
    connect(threadb, SIGNAL(finished()), threadb, SLOT(deleteLater()));
    threadb->start();
}


QTEST_MAIN(TestQueue)
#include "main.moc"
