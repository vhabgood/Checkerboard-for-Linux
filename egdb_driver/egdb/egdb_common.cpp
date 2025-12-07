/*
* Copyright (C) 2017-2021 by E. Gilbert
*
* This file is part of the egdb_intl library.
*
* This library is free software; you can redistribute it and/or modify it
* under the terms of the Boost Software License, Version 1.0.
*/

#include <cstdio>
#include <cstring>
#include <cctype>
#include <mutex>

#include "egdb/egdb_intl.h"
#include "egdb/egdb_common.h"
#include "egdb/crc.h"
#include "engine/bitcount.h"
#include "engine/board.h"
#include "engine/reverse.h"
#include "egdb/thread_safety.h"


#define CACHE_LINE_SIZE 64


namespace egdb_interface {

	/** a list of all open database drivers. */
	EGDB_DRIVER *driver_list = 0;

	/** protects access to driver_list. */
	LOCK_T driver_list_lock = nullptr;

	/** number of drivers in driver_list. */
	int num_drivers = 0;

	// These are implemented in other files and will be linked.
	extern int egdb_wld_lookup(EGDB_DRIVER *driver, const EGDB_POSITION *pos, EGDB_ERR *err_code);
	extern int egdb_dtw_lookup(EGDB_DRIVER *driver, const EGDB_POSITION *pos, EGDB_ERR *err_code);
	extern int egdb_mtc_lookup(EGDB_DRIVER *driver, const EGDB_POSITION *pos, EGDB_ERR *err_code);
	extern EGDB_ERR egdb_close_wld(EGDB_DRIVER *driver);
	extern EGDB_ERR egdb_close_dtw(EGDB_DRIVER *driver);
	extern EGDB_ERR egdb_close_mtc(EGDB_DRIVER *driver);
	extern unsigned int egdb_get_max_pieces_wld(EGDB_DRIVER *handle);
	extern unsigned int egdb_get_max_pieces_dtw(EGDB_DRIVER *handle);
	extern unsigned int egdb_get_max_pieces_mtc(EGDB_DRIVER *handle);
	extern unsigned int egdb_get_info_wld(EGDB_DRIVER *driver,
									  unsigned int num_pieces,
									  EGDB_INFO *info,
									  unsigned int max_info);
	extern unsigned int egdb_get_info_dtw(EGDB_DRIVER *driver,
									  unsigned int num_pieces,
									  EGDB_INFO *info,
									  unsigned int max_info);
	extern unsigned int egdb_get_info_mtc(EGDB_DRIVER *driver,
									  unsigned int num_pieces,
									  EGDB_INFO *info,
									  unsigned int max_info);

	/**
	* Verifies that a driver handle is valid.
	* @param handle a database handle.
	* @return a pointer to the EGDB_DRIVER if the handle is valid, else 0.
	*/
	EGDB_DRIVER *egdb_verify(EGDB_DRIVER_HANDLE handle)
	{
		EGDB_DRIVER *d;

		if (driver_list_lock == nullptr) {
			init_lock(driver_list_lock);
		}

		lock(driver_list_lock);
		d = driver_list;
		while (d) {
			if (d->handle == handle)
				break;
			d = d->next;
		}
		unlock(driver_list_lock);
		return(d);
	}

	int egdb_lookup(
		EGDB_DRIVER_HANDLE handle,
		const EGDB_POSITION *pos,
		EGDB_ERR *err_code)
	{
		EGDB_DRIVER *driver;

		driver = egdb_verify(handle);
		if (!driver) {
			*err_code = EGDB_INVALID_HANDLE;
			return(EGDB_UNKNOWN);
		}

		// check that num pieces not > max pieces in this db.
		if (bit_pop_count64(pos->black_pieces | pos->white_pieces) > driver->max_pieces) {
			*err_code = EGDB_NUM_PIECES_TOO_LARGE;
			return(EGDB_UNKNOWN);
		}

		switch (driver->db_type) {
		case EGDB_WLD_TUN_V1:
		case EGDB_WLD_TUN_V2:
		case EGDB_WLD_RUNLEN:
			return(egdb_wld_lookup(driver, pos, err_code));
		case EGDB_DTW:
			return(egdb_dtw_lookup(driver, pos, err_code));
		case EGDB_MTC_RUNLEN:
			return(egdb_mtc_lookup(driver, pos, err_code));
		default:
			*err_code = EGDB_INVALID_DB_TYPE;
			return(EGDB_UNKNOWN);
		}
	}

	EGDB_ERR egdb_close(EGDB_DRIVER_HANDLE handle)
	{
		EGDB_DRIVER *driver, *d, *d_prev;
		EGDB_ERR err;

		driver = egdb_verify(handle);
		if (!driver)
			return(EGDB_INVALID_HANDLE);

		// remove this driver from the driver list.
		lock(driver_list_lock);
		d_prev = 0;
		d = driver_list;
		while (d) {
			if (d == driver) {
				if (d_prev)
					d_prev->next = d->next;
				else
					driver_list = d->next;
				num_drivers--;
				break;
			}
			d_prev = d;
			d = d->next;
		}
		unlock(driver_list_lock);

		if (!d)
			return(EGDB_INVALID_HANDLE);

		// close the driver
		switch (driver->db_type) {
		case EGDB_WLD_TUN_V1:
		case EGDB_WLD_TUN_V2:
		case EGDB_WLD_RUNLEN:
			err = egdb_close_wld(driver);
			break;
		case EGDB_DTW:
			err = egdb_close_dtw(driver);
			break;
		case EGDB_MTC_RUNLEN:
			err = egdb_close_mtc(driver);
			break;
		default:
			err = EGDB_INVALID_DB_TYPE;
			break;
		}

		return(err);
	}

	unsigned int egdb_get_max_pieces(EGDB_DRIVER_HANDLE handle)
	{
		EGDB_DRIVER *driver;

		driver = egdb_verify(handle);
		if (!driver)
			return(0);

		switch (driver->db_type) {
		case EGDB_WLD_TUN_V1:
		case EGDB_WLD_TUN_V2:
		case EGDB_WLD_RUNLEN:
			return(egdb_get_max_pieces_wld(driver));
		case EGDB_DTW:
			return(egdb_get_max_pieces_dtw(driver));
		case EGDB_MTC_RUNLEN:
			return(egdb_get_max_pieces_mtc(driver));
		default:
			return(0);
		}
	}

	unsigned int egdb_get_info(
		EGDB_DRIVER_HANDLE handle,
		unsigned int num_pieces,
		EGDB_INFO *info,
		unsigned int max_info)
	{
		EGDB_DRIVER *driver;

		driver = egdb_verify(handle);
		if (!driver)
			return(0);

		switch (driver->db_type) {
		case EGDB_WLD_TUN_V1:
		case EGDB_WLD_TUN_V2:
		case EGDB_WLD_RUNLEN:
			return(egdb_get_info_wld(driver, num_pieces, info, max_info));
		case EGDB_DTW:
			return(egdb_get_info_dtw(driver, num_pieces, info, max_info));
		case EGDB_MTC_RUNLEN:
			return(egdb_get_info_mtc(driver, num_pieces, info, max_info));
		default:
			return(0);
		}
	}


	/**
	* Reads from a file that is memory-mapped, or for which reads must be a
	* multiple of a page size. This is for Windows; for other OS's it does
	* a normal read.
	* @param buffer a buffer to hold the file data.
	* @param size the number of bytes to read.
	* @param file the file handle.
	* @return 0 if success, else an error code.
	*/
	int read_file(void *buffer, unsigned int size, FILE_HANDLE file)
	{
#if defined(_MSC_VER) && defined(USE_WIN_API)
		DWORD bytes_read;

		if (!ReadFile(file, buffer, size, &bytes_read, 0))
			return(1);
		if (bytes_read != size)
			return(1);
#else
		if (std::fread(buffer, size, 1, file) != 1)
			return(1);
#endif
		return(0);
	}

}	// namespace egdb_interface