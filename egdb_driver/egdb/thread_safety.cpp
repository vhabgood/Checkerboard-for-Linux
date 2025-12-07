#include "egdb/thread_safety.h"
#include "egdb/platform.h"
#include <mutex>

namespace egdb_interface {

void init_lock(LOCK_T &lock_t)
{
    lock_t = new std::mutex();
}

void destroy_lock(LOCK_T &lock_t)
{
    delete lock_t;
    lock_t = nullptr;
}

void lock(LOCK_T &lock_t)
{
    lock_t->lock();
}

void unlock(LOCK_T &lock_t)
{
    lock_t->unlock();
}

} // namespace egdb_interface
