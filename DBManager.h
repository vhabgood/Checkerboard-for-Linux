#pragma once

#include <QObject>
#include <QMutex>
#include "egdb_driver/egdb/egdb.h" // Include Kingsrow EGDB driver header
#include "checkers_types.h" // For bitboard_pos (will be converted to EGDB_POSITION)

class DBManager : public QObject
{
    Q_OBJECT

public:
    static DBManager* instance();
    ~DBManager();

    // db_init now initializes the Kingsrow EGDB driver handles
    int db_init(int suggestedMB, char out[256], const char* EGTBdirectory);
    
    // dblookup (for WLD) now uses the Kingsrow EGDB_POSITION and returns EGDB_ERR
    int dblookup(const EGDB_POSITION *pos, EGDB_ERR *err_code);
    
    int db_exit();
    
    // These functions might need re-evaluation or removal depending on Kingsrow API
    int64_t getDatabaseSize(int bm, int bk, int wm, int wk, int bmrank, int wmrank);
    void getInfoString(char *str);
    int getCacheSize();
    QString getEGDBStatus() const;

private:
    explicit DBManager(QObject *parent = nullptr);
    static DBManager* m_instance;
    QMutex m_mutex;
    bool m_shutdown_has_been_called = false;

    // Kingsrow EGDB driver handles
    EGDB_DRIVER_HANDLE m_wld_handle;
    EGDB_DRIVER_HANDLE m_mtc_handle;
    
public:
    // Public MTC lookup function, now uses Kingsrow EGDB_POSITION and returns EGDB_ERR
    int dblookup_mtc(const EGDB_POSITION *pos, EGDB_ERR *err_code);
    
};