#ifndef EGDBDRIVER_H
#define EGDBDRIVER_H

#include <QObject>
#include <string>
#include <QMutex>
#include "core_types.h"
#include "egdb_driver/egdb/egdb_types.h"
#include "egdb_driver/egdb/egdb.h"


class EGDBDriver : public QObject
{
    Q_OBJECT
public:
    static EGDBDriver* instance();
    ~EGDBDriver();

    bool init(const std::string& db_path, int cache_size_mb);
    int lookup(bitboard_pos* pos, int color, bool& is_win, bool& is_loss, bool& is_draw);
    void close();

private:
    explicit EGDBDriver(QObject *parent = nullptr);
    static EGDBDriver* m_instance;
    
    QMutex m_mutex;
    EGDB_DRIVER* m_wld_handle;
    EGDB_DRIVER* m_mtc_handle;
};

#endif // EGDBDRIVER_H
