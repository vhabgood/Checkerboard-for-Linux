#include "egdb_driver.h"
#include "checkers_types.h"
#include <QDebug>
#include "egdb_driver/egdb/egdb_intl.h"

using namespace egdb_interface;

EGDBDriver* EGDBDriver::m_instance = nullptr;

EGDBDriver* EGDBDriver::instance()
{
    if (!m_instance) {
        m_instance = new EGDBDriver();
    }
    return m_instance;
}

EGDBDriver::EGDBDriver(QObject *parent)
    : QObject(parent), m_wld_handle(nullptr), m_mtc_handle(nullptr)
{
}

EGDBDriver::~EGDBDriver()
{
    close();
}

bool EGDBDriver::init(const std::string& db_path, int cache_size_mb)
{
    QMutexLocker locker(&m_mutex);
    EGDB_ERR err;

    // Open WLD databases
    m_wld_handle = egdb_open(db_path.c_str(), (unsigned int)cache_size_mb, EGDB_WLD_RUNLEN, &err);
    if (err != EGDB_ERR_NORMAL) {
        qWarning() << "Failed to open WLD databases, error: " << (int)err;
        m_wld_handle = nullptr;
    } else {
        qDebug() << "WLD databases opened successfully.";
    }

    // Open MTC databases
    m_mtc_handle = egdb_open(db_path.c_str(), (unsigned int)cache_size_mb, EGDB_MTC_RUNLEN, &err);
    if (err != EGDB_ERR_NORMAL) {
        qWarning() << "Failed to open MTC databases, error: " << (int)err;
        m_mtc_handle = nullptr;
    } else {
        qDebug() << "MTC databases opened successfully.";
    }

    return m_wld_handle || m_mtc_handle;
}

int EGDBDriver::lookup(bitboard_pos *pos, int color, bool& is_win, bool& is_loss, bool& is_draw)
{
    QMutexLocker locker(&m_mutex);
    EGDB_POSITION egdb_pos;
    egdb_pos.black_pieces = pos->bm | pos->bk;
    egdb_pos.white_pieces = pos->wm | pos->wk;
    egdb_pos.king = pos->bk | pos->wk;
    egdb_pos.stm = (color == CB_BLACK) ? EGDB_BLACK_TO_MOVE : EGDB_WHITE_TO_MOVE;

    EGDB_ERR err;
    int result = EGDB_UNKNOWN;

    // Try MTC lookup first
    if (m_mtc_handle) {
        result = egdb_lookup(m_mtc_handle, &egdb_pos, &err);
        // MTC result is a number of moves. We can infer WLD from it.
        if (result > 0) { // Win is possible
            is_win = true;
        }
        return result;
    }

    // If MTC fails or is not available, try WLD
    if (m_wld_handle) {
        result = egdb_lookup(m_wld_handle, &egdb_pos, &err);
        is_win = (result == EGDB_WIN);
        is_loss = (result == EGDB_LOSS);
        is_draw = (result == EGDB_DRAW);
        return result;
    }

    return EGDB_UNKNOWN;
}

void EGDBDriver::close()
{
    QMutexLocker locker(&m_mutex);
    if (m_wld_handle) {
        egdb_close(m_wld_handle);
        m_wld_handle = nullptr;
    }
    if (m_mtc_handle) {
        egdb_close(m_mtc_handle);
        m_mtc_handle = nullptr;
    }
}
