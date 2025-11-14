#include "AutoThreadWorker.h"
#include <QThread>
#include <QDebug>
#include <QMutexLocker>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>
#include "cb_interface.h"


AutoThreadWorker::AutoThreadWorker(QObject *parent) : QObject(parent),
    m_mode(Idle),
    m_startMatchFlag(false),
    m_matchGameNumber(0),
    m_matchMoveCount(0),
    m_matchGameOver(false)
{
    m_abortRequested.storeRelaxed(0);
}

AutoThreadWorker::~AutoThreadWorker()
{
}