#include "UserBookManager.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include "c_logic.h" // For boardtobitboard, bitboardtoboard8

UserBookManager::UserBookManager()
    : m_userbooknum(0),
      m_userbookcur(0)
{
}

void UserBookManager::loadUserBook(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open user book file for reading:" << filename;
        m_userbooknum = 0;
        m_userbookcur = 0;
        return;
    }

    QDataStream in(&file);
    m_userbooknum = 0;
    while (!in.atEnd() && m_userbooknum < MAXUSERBOOK) {
        userbookentry entry;
        in.readRawData(reinterpret_cast<char*>(&entry.position), sizeof(pos));
        in.readRawData(reinterpret_cast<char*>(&entry.move), sizeof(CBmove));
        m_userbook[m_userbooknum++] = entry;
    }
    file.close();
    m_userbookcur = 0;
    qDebug() << "Loaded" << m_userbooknum << "entries from user book:" << filename;
}

void UserBookManager::saveUserBook(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open user book file for writing:" << filename;
        return;
    }

    QDataStream out(&file);
    for (int i = 0; i < m_userbooknum; ++i) {
        out.writeRawData(reinterpret_cast<const char*>(&m_userbook[i].position), sizeof(pos));
        out.writeRawData(reinterpret_cast<const char*>(&m_userbook[i].move), sizeof(CBmove));
    }
    file.close();
    qDebug() << "Saved" << m_userbooknum << "entries to user book:" << filename;
}

void UserBookManager::addMoveToUserBook(const Board8x8 board, const CBmove& move)
{
    if (m_userbooknum >= MAXUSERBOOK) {
        qWarning() << "User book size limit reached!";
        return;
    }

    pos currentPos;
boardtobitboard(&board, &currentPos);
    int existingIndex = -1;
    for (int i = 0; i < m_userbooknum; ++i) {
        if (m_userbook[i].position.bm == currentPos.bm &&
            m_userbook[i].position.bk == currentPos.bk &&
            m_userbook[i].position.wm == currentPos.wm &&
            m_userbook[i].position.wk == currentPos.wk) {
            existingIndex = i;
            break;
        }
    }

    if (existingIndex != -1) {
        // Overwrite existing entry
        m_userbook[existingIndex].move = move;
        qDebug() << "Replaced move in user book at index" << existingIndex;
    } else {
        // Add new entry
        m_userbook[m_userbooknum].position = currentPos;
        m_userbook[m_userbooknum].move = move;
        m_userbooknum++;
        qDebug() << "Added move to user book. Total entries:" << m_userbooknum;
    }
}

bool UserBookManager::lookupMove(const Board8x8 board, int color, int gametype, CBmove* bookMove) const
{
    pos currentPos;
    boardtobitboard(&board, &currentPos);

    for (int i = 0; i < m_userbooknum; ++i) {
        if (m_userbook[i].position.bm == currentPos.bm &&
            m_userbook[i].position.bk == currentPos.bk &&
            m_userbook[i].position.wm == currentPos.wm &&
            m_userbook[i].position.wk == currentPos.wk) {
            *bookMove = m_userbook[i].move;
            qDebug() << "Found move in user book.";
            return true;
        }
    }
    return false;
}

void UserBookManager::deleteCurrentEntry()
{
    if (m_userbooknum == 0 || m_userbookcur < 0 || m_userbookcur >= m_userbooknum) {
        qWarning() << "No entry to delete or invalid current index.";
        return;
    }

    for (int i = m_userbookcur; i < m_userbooknum - 1; ++i) {
        m_userbook[i] = m_userbook[i+1];
    }
    m_userbooknum--;

    if (m_userbookcur >= m_userbooknum && m_userbooknum > 0) {
        m_userbookcur = m_userbooknum - 1;
    } else if (m_userbooknum == 0) {
        m_userbookcur = 0;
    }
    qDebug() << "Deleted entry from user book. Total entries:" << m_userbooknum;
}

void UserBookManager::navigateToNextEntry()
{
    if (m_userbooknum > 0) {
        m_userbookcur = (m_userbookcur + 1) % m_userbooknum;
        qDebug() << "Navigated to next user book entry:" << m_userbookcur;
    }
}

void UserBookManager::navigateToPreviousEntry()
{
    if (m_userbooknum > 0) {
        m_userbookcur = (m_userbookcur - 1 + m_userbooknum) % m_userbooknum;
        qDebug() << "Navigated to previous user book entry:" << m_userbookcur;
    }
}

void UserBookManager::resetNavigation()
{
    m_userbookcur = 0;
    qDebug() << "User book navigation reset.";
}

const userbookentry* UserBookManager::getCurrentEntry() const
{
    if (m_userbooknum > 0 && m_userbookcur >= 0 && m_userbookcur < m_userbooknum) {
        return &m_userbook[m_userbookcur];
    }
    return nullptr;
}