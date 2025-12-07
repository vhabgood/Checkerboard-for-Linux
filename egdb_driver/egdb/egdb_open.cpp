/*
* Copyright (C) 2017-2021 by E. Gilbert
*
* This file is part of the egdb_intl library.
*
* This library is free software; you can redistribute it and/or modify it
* under the terms of the Boost Software License, Version 1.0.
*/

#include <cstddef> // For NULL
#include <cstdlib> // For std::calloc, std::free
#include <cstring> // For std::strncpy
#include "egdb/egdb.h"
#include "egdb/egdb_common.h"
#include "egdb/egdb_intl.h"
#include "engine/bicoef.h"
#include "engine/board.h"
#include "engine/reverse.h"
#include "engine/bitcount.h"
#include "builddb/compression_tables.h"
#include "builddb/indexing.h"
#include "egdb/thread_safety.h"
#include "db_stats.h"
#include "log.h"


namespace egdb_interface {

	extern LOCK_T driver_list_lock;
	extern EGDB_DRIVER *driver_list;
	extern int num_drivers;

	void build_man_index_base(void);
	extern EGDB_ERR egdb_open_wld_tun_v1(EGDB_DRIVER *driver);
	extern EGDB_ERR egdb_open_wld_tun_v2(EGDB_DRIVER *driver);
	extern EGDB_ERR egdb_open_wld_runlen(EGDB_DRIVER *driver);
	extern EGDB_ERR egdb_open_dtw(EGDB_DRIVER *driver);
	extern EGDB_ERR egdb_open_mtc_runlen(EGDB_DRIVER *driver);


	static unsigned int get_cache_size(unsigned int cache_size_mb)
	{
		unsigned int size;

		if (cache_size_mb == 0)
			size = EGDB_DEFAULT_CACHE_SIZE;
		else
			size = cache_size_mb * 1024 * 1024;
		return(size);
	}

	static EGDB_DRIVER_HANDLE egdb_add_driver(EGDB_DRIVER *driver)
	{
		if (driver_list_lock == nullptr) {
			init_lock(driver_list_lock);
		}
		lock(driver_list_lock);
		driver->handle = (EGDB_DRIVER_HANDLE)driver;
		driver->next = driver_list;
		driver_list = driver;
		num_drivers++;
		unlock(driver_list_lock);
		return(driver->handle);
	}

	EGDB_DRIVER_HANDLE egdb_open(
		const char *db_path,
		unsigned int cache_size_mb,
		EGDB_TYPE db_type,
		EGDB_ERR *err_code)
	{
		EGDB_DRIVER *driver;
		EGDB_ERR err = EGDB_ERR_NORMAL;
	
		driver = (EGDB_DRIVER *)std::calloc(1, sizeof(EGDB_DRIVER));
		if (!driver) {
			if (err_code) *err_code = EGDB_NO_MEM;
			return(NULL);
		}
	
		driver->max_pieces = MAXPIECE; 
		std::strncpy(driver->path, db_path, sizeof(driver->path) - 1);
		driver->cache_size = get_cache_size(cache_size_mb);
		driver->db_type = db_type;
		driver->bitboard_type = EGDB_NORMAL;
	
		// initialize the common tables (bicoef, etc.)
		initbicoef();
		init_board();
		init_reverse();
		init_bitcount();
		init_compression_tables();
		build_man_index_base();
	
		// now open the specific type of db
		switch (driver->db_type) {
		case EGDB_WLD_RUNLEN:
			err = egdb_open_wld_runlen(driver);
			break;
		case EGDB_MTC_RUNLEN:
			err = egdb_open_mtc_runlen(driver);
			break;
		default:
			err = EGDB_INVALID_DB_TYPE;
			break;
		}
	
		if (err != EGDB_ERR_NORMAL) {
			std::free(driver);
			if (err_code) *err_code = err;
			return(NULL);
		}
	
		if (err_code) *err_code = EGDB_ERR_NORMAL;
		return(egdb_add_driver(driver));
	}
	EGDB_API EGDB_DRIVER *egdb_open(
		EGDB_BITBOARD_TYPE bitboard_type,
		int pieces,
		int cache_mb,
		const char *directory,
		void (*msg_fn)(char *))
	{
		EGDB_TYPE identified_type;
		int identified_max_pieces = 0;
		EGDB_ERR err;

		if (egdb_identify(directory, &identified_type, &identified_max_pieces) != 0) {
            log_c(LOG_LEVEL_ERROR, "egdb_open: egdb_identify failed for directory: %s", directory);
			if (msg_fn) msg_fn((char*)"EGDB_UNKNOWN_DB_TYPE");
			return(NULL);
		}

		EGDB_DRIVER *driver = (EGDB_DRIVER*)egdb_open(directory, cache_mb, identified_type, &err);
		if (driver) {
			driver->bitboard_type = bitboard_type;
			driver->max_pieces = pieces;
			driver->msg_fn = msg_fn;
		} else {
            log_c(LOG_LEVEL_ERROR, "egdb_open: internal egdb_open failed with error: %d", (int)err);
			if (msg_fn) msg_fn((char*)"Failed to open identified EGDB.");
		}
		return driver;
	}


	unsigned int get_total_num_dbs(EGDB_DRIVER *driver)
	{
		switch (driver->db_type) {
		case EGDB_WLD_TUN_V1:
		case EGDB_WLD_TUN_V2:
		case EGDB_WLD_RUNLEN:
			//return(get_total_num_wld_dbs());
		case EGDB_DTW:
			//return(get_total_num_dtw_dbs());
		case EGDB_MTC_RUNLEN:
			//return(get_total_num_mtc_dbs());
		default:
			return(0);
		}
	}

}	// namespace egdb_interface