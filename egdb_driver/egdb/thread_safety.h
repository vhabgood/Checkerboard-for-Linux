#ifndef THREAD_SAFETY_H
#define THREAD_SAFETY_H

#include "engine/lock.h" // Include the header defining LOCK_T

namespace egdb_interface {

// Forward declarations for locking functions that take LOCK_T&
void init_lock(LOCK_T &lock_t);
void destroy_lock(LOCK_T &lock_t);
void lock(LOCK_T &lock_t);
void unlock(LOCK_T &lock_t);

} // namespace egdb_interface

#endif // THREAD_SAFETY_H