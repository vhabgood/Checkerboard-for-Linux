#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QObject>
#include <QSharedMemory>

class SingleApplication : public QObject
{
    Q_OBJECT

public:
    explicit SingleApplication(QObject *parent = nullptr);
    bool isAnotherInstanceRunning();
    void release();

private:
    QSharedMemory sharedMemory;
};

#endif // SINGLEAPPLICATION_H
