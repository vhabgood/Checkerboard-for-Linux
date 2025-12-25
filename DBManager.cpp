#include "core_types.h"
#include "DBManager.h"
#include <QMutexLocker>
#include <QDebug>
#include <QDir>
#include "log.h"

#include "c_logic.h"
#include "egdb_driver/egdb/egdb_intl.h"

using namespace egdb_interface;



DBManager* DBManager::m_instance = nullptr;

DBManager* DBManager::instance()
{
    if (!m_instance) {
        m_instance = new DBManager();
    }
    return m_instance;
}

DBManager::DBManager(QObject *parent) : QObject(parent),
    m_wld_handle(nullptr),
    m_mtc_handle(nullptr)
{
}

DBManager::~DBManager()
{
    db_exit();
}



int DBManager::db_init(int suggestedMB, char out[256], const char* EGTBdirectory)
{
    QMutexLocker locker(&m_mutex);
    
    // Clear any previous EGDB status flags and set INIT_START
    clearProgramStatusWordFlags(0xFFFFFFFF); // Clear all flags to start fresh
    updateProgramStatusWord(STATUS_APP_START | STATUS_EGDB_INIT_START);
    // Normalize directory path (remove trailing slash)
    QString normalizedPath = QString::fromUtf8(EGTBdirectory);
    if (normalizedPath.isEmpty() || !QDir(normalizedPath).exists()) {
        log_c(LOG_LEVEL_ERROR, "DBManager::db_init: EGDB directory does not exist: %s", EGTBdirectory);
        updateProgramStatusWord(STATUS_EGDB_INIT_FAIL);
        return 0;
    }
    std::string finalPath = normalizedPath.toStdString();
    log_c(LOG_LEVEL_INFO, "DBManager::db_init: Initializing EGDB from directory: %s", finalPath.c_str());

    if (suggestedMB < 4) suggestedMB = 4;

    // Placeholder for actual Kingsrow EGDB driver initialization
    // For WLD database
    EGDB_ERR wld_err = EGDB_ERR_NORMAL;
    // We assume an 8x8 board (32 squares)
    // The Kingsrow EGDB driver takes a 'pieces' argument, which refers to the max number of pieces for the DB
    // The egdb_open function also takes bitboard_type and a message function.
    m_wld_handle = egdb_open(EGDB_NORMAL, MAXPIECE, suggestedMB, finalPath.c_str(), [](char* msg){ log_c(LOG_LEVEL_DEBUG, "EGDB WLD: %s", msg); });
    
    if (m_wld_handle == nullptr) {
        log_c(LOG_LEVEL_ERROR, "DBManager::db_init: Failed to open WLD EGDB.");
        updateProgramStatusWord(STATUS_EGDB_INIT_FAIL);
        // Attempt to open MTC anyway, as it might be a separate issue
    } else {
        log_c(LOG_LEVEL_INFO, "DBManager::db_init: WLD EGDB opened successfully.");
    }

    // For MTC database
    EGDB_ERR mtc_err = EGDB_ERR_NORMAL;
    m_mtc_handle = egdb_open(finalPath.c_str(), (unsigned int)suggestedMB, EGDB_MTC_RUNLEN, &mtc_err);

    if (mtc_err != EGDB_ERR_NORMAL) {
        log_c(LOG_LEVEL_ERROR, "DBManager::db_init: Failed to open MTC EGDB with error: %d", (int)mtc_err);
        m_mtc_handle = nullptr;
    } else {
        log_c(LOG_LEVEL_INFO, "DBManager::db_init: MTC EGDB opened successfully.");
    }


    if (m_wld_handle == nullptr && m_mtc_handle == nullptr) {
        updateProgramStatusWord(STATUS_EGDB_INIT_FAIL);
        log_c(LOG_LEVEL_ERROR, "DBManager::db_init: Both WLD and MTC EGDB failed to initialize.");
        return 0; // Indicates failure
    }
    
    updateProgramStatusWord(STATUS_EGDB_INIT_OK);
    log_c(LOG_LEVEL_INFO, "DBManager::db_init: EGDB initialization successful (PSW: 0x%08X).", getProgramStatusWord());

    // Returning 1 for success, the actual maxpieces will be queried from the driver if needed.
    // The previous return value was maxpieces, but with the new driver, this is less relevant
    // as maxpieces is passed to egdb_open.
    return 1; 
}

int DBManager::dblookup(const EGDB_POSITION *pos, EGDB_ERR *err_code)
{
    QMutexLocker locker(&m_mutex);
    if (m_wld_handle == nullptr) {
        *err_code = EGDB_DB_NOT_LOADED;
        return EGDB_UNKNOWN;
    }
    
    updateProgramStatusWord(STATUS_EGDB_LOOKUP_ATTEMPT);

    // Pass bitboards directly to the driver. The driver logic handles orientation.
    EGDB_POSITION lookup_pos;
    lookup_pos.black_pieces = pos->black_pieces;
    lookup_pos.white_pieces = pos->white_pieces;
    lookup_pos.king = pos->king;
    
    // Map side-to-move constants: App(4/8) -> Driver(1/0)
    // EGDB_WHITE_TO_MOVE = 1, EGDB_BLACK_TO_MOVE = 0
    if (pos->stm == CB_WHITE) lookup_pos.stm = EGDB_WHITE_TO_MOVE;
    else if (pos->stm == CB_BLACK) lookup_pos.stm = EGDB_BLACK_TO_MOVE;
    else lookup_pos.stm = pos->stm; // Fallback

    int result = egdb_lookup(m_wld_handle, &lookup_pos, err_code);

    if (*err_code == EGDB_ERR_NORMAL) {
        updateProgramStatusWord(STATUS_EGDB_LOOKUP_HIT);
        if (result == EGDB_WIN) updateProgramStatusWord(STATUS_EGDB_WIN_RESULT);
        else if (result == EGDB_LOSS) updateProgramStatusWord(STATUS_EGDB_LOSS_RESULT);
        else if (result == EGDB_DRAW) updateProgramStatusWord(STATUS_EGDB_DRAW_RESULT);
        else updateProgramStatusWord(STATUS_EGDB_UNKNOWN_RESULT);
    } else {
        updateProgramStatusWord(STATUS_EGDB_LOOKUP_MISS);
        if (*err_code == EGDB_NUM_PIECES_OUT_OF_BOUNDS) updateProgramStatusWord(STATUS_EGDB_LOOKUP_OUT_OF_BOUNDS);
        // Add other error flag updates as needed
    }
    return result;
}

int DBManager::db_exit()
{
    QMutexLocker locker(&m_mutex);
    if (m_shutdown_has_been_called) {
        return 1;
    }
    m_shutdown_has_been_called = true;
    
    if (m_wld_handle) {
        egdb_close(m_wld_handle);
        m_wld_handle = nullptr;
        log_c(LOG_LEVEL_INFO, "DBManager::db_exit: WLD EGDB closed.");
    }
    if (m_mtc_handle) {
        egdb_close(m_mtc_handle);
        m_mtc_handle = nullptr;
        log_c(LOG_LEVEL_INFO, "DBManager::db_exit: MTC EGDB closed.");
    }

	return 1;
}

int DBManager::getCacheSize()
{
    QMutexLocker locker(&m_mutex);
    // This function needs to be re-implemented to query the Kingsrow driver for cache size
    // For now, return 0 or a placeholder
    return 0;
}



int DBManager::dblookup_mtc(const EGDB_POSITION *pos, EGDB_ERR *err_code)
{
    QMutexLocker locker(&m_mutex);
    if (m_mtc_handle == nullptr) {
        *err_code = EGDB_DB_NOT_LOADED;
        return MTC_UNKNOWN;
    }
    
    updateProgramStatusWord(STATUS_EGDB_LOOKUP_ATTEMPT);

    // Pass bitboards directly to the driver. The driver logic handles orientation.
    EGDB_POSITION lookup_pos;
    lookup_pos.black_pieces = pos->black_pieces;
    lookup_pos.white_pieces = pos->white_pieces;
    lookup_pos.king = pos->king;
    
    // Map side-to-move constants: App(4/8) -> Driver(1/0)
    // EGDB_WHITE_TO_MOVE = 1, EGDB_BLACK_TO_MOVE = 0
    if (pos->stm == CB_WHITE) lookup_pos.stm = EGDB_WHITE_TO_MOVE;
    else if (pos->stm == CB_BLACK) lookup_pos.stm = EGDB_BLACK_TO_MOVE;
    else lookup_pos.stm = pos->stm; // Fallback

    int result = egdb_lookup(m_mtc_handle, &lookup_pos, err_code);

    if (*err_code == EGDB_ERR_NORMAL) {
        updateProgramStatusWord(STATUS_EGDB_LOOKUP_HIT);
        log_c(LOG_LEVEL_DEBUG, "DBManager::dblookup_mtc: HIT! Result=%d plies", result);
    } else {
        updateProgramStatusWord(STATUS_EGDB_LOOKUP_MISS);
        log_c(LOG_LEVEL_DEBUG, "DBManager::dblookup_mtc: MISS! Error=%d", (int)*err_code);
        if (*err_code == EGDB_NUM_PIECES_OUT_OF_BOUNDS) updateProgramStatusWord(STATUS_EGDB_LOOKUP_OUT_OF_BOUNDS);
        // Add other error flag updates as needed
    }
    return result;
}

void DBManager::getInfoString(char *str)
{
    QMutexLocker locker(&m_mutex);
    // This function needs to be re-implemented to query the Kingsrow driver for info
    // For now, it will return a dummy string
    sprintf(str, "\nDatabase Details: Kingsrow EGDB driver active.");
}

int64_t DBManager::getDatabaseSize(int bm, int bk, int wm, int wk, int bmrank, int wmrank)
{
    QMutexLocker locker(&m_mutex);
    // This function needs to be re-implemented to query the Kingsrow driver if such info is available
    // For now, return 0
    return 0;
}
	
	QString DBManager::getEGDBStatus() const {
	    QString status;
	    uint32_t psw = getProgramStatusWord();	
	    if (psw & STATUS_CRITICAL_ERROR) status += "CRITICAL_ERROR; ";
	    if (psw & STATUS_FILE_IO_ERROR) status += "FILE_IO_ERROR; ";
	
	    if (psw & STATUS_EGDB_INIT_START) status += "EGDB_INIT_START; ";
	    if (psw & STATUS_EGDB_INIT_OK) status += "EGDB_INIT_OK; ";
	    if (psw & STATUS_EGDB_INIT_FAIL) status += "EGDB_INIT_FAIL; ";
	
	    if (psw & STATUS_EGDB_LOOKUP_ATTEMPT) status += "EGDB_LOOKUP_ATTEMPT; ";
	    if (psw & STATUS_EGDB_LOOKUP_HIT) status += "EGDB_LOOKUP_HIT; ";
	    if (psw & STATUS_EGDB_LOOKUP_MISS) status += "EGDB_LOOKUP_MISS; ";
	    if (psw & STATUS_EGDB_UNEXPECTED_VALUE) status += "EGDB_UNEXPECTED_VALUE; ";
	    if (psw & STATUS_EGDB_LOOKUP_OUT_OF_BOUNDS) status += "EGDB_LOOKUP_OUT_OF_BOUNDS; ";
	    if (psw & STATUS_EGDB_LOOKUP_NOT_PRESENT) status += "EGDB_LOOKUP_NOT_PRESENT; ";
	    if (psw & STATUS_EGDB_LOOKUP_INVALID_INDEX) status += "EGDB_LOOKUP_INVALID_INDEX; ";
	    if (psw & STATUS_EGDB_SINGLE_VALUE_HIT) status += "EGDB_SINGLE_VALUE_HIT; ";
	    if (psw & STATUS_EGDB_WIN_RESULT) status += "EGDB_WIN_RESULT; ";
	    if (psw & STATUS_EGDB_LOSS_RESULT) status += "EGDB_LOSS_RESULT; ";
	    if (psw & STATUS_EGDB_DRAW_RESULT) status += "EGDB_DRAW_RESULT; ";
	    if (psw & STATUS_EGDB_UNKNOWN_RESULT) status += "EGDB_UNKNOWN_RESULT; ";
	    if (psw & STATUS_EGDB_DISK_READ_ERROR) status += "EGDB_DISK_READ_ERROR; ";
	    if (psw & STATUS_EGDB_DECODE_ERROR) status += "EGDB_DECODE_ERROR; ";
	    if (psw & STATUS_APP_START) status += "APP_START; ";
	
	    if (status.isEmpty()) {
	        return "No specific EGDB status flags set.";
	    } else {
	        return status.trimmed(); // Remove trailing space if any
	    }
	}
	