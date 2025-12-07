#ifndef LOCK_H
#define LOCK_H

#if defined(_MSC_VER)
#if defined(USE_WIN_API)
#include "windows.h"
#else
#include <thread>
#include <mutex>
#include <condition_variable>
#endif
#else
#include <thread>
#include <mutex>
#include <condition_variable>
#endif


namespace egdb_interface {

#ifdef USE_WIN_API
typedef CRITICAL_SECTION LOCK_T;
#else
typedef std::mutex *LOCK_T;
#endif


// forward references
void init_lock(LOCK_T &lock);
void destroy_lock(LOCK_T &lock);
void lock(LOCK_T &lock);
void unlock(LOCK_T &lock);
}	// namespace egdb_interface

#endif
