/*
* Copyright (C) 2017-2021 by E. Gilbert
*
* This file is part of the egdb_intl library.
*
* This library is free software; you can redistribute it and/or modify it
* under the terms of the Boost Software License, Version 1.0.
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "egdb/egdb_intl.h"
#include "egdb/egdb_common.h"
#include "egdb/crc.h"
#include "log.h"

namespace egdb_interface {

	typedef struct {
		EGDB_TYPE type;
		char const *filename;
		unsigned int num_pieces;
		bool dtw_w_only;
		bool contains_le_pieces;
		unsigned int crc;
	} EGDB_FIND_INFO;

	/**
	* A table of info used to identify a database by filename and crc.
	* This is from generator version 1.25.
	*/
	static const EGDB_FIND_INFO egdb_find_table[] = {

		// 8 piece dtw databases.
		{ EGDB_DTW, "dtw-8-w.bin", 8, true, false, 0x51da337b },
		{ EGDB_DTW, "dtw-8-b.bin", 8, true, false, 0x86915233 },

		// 7 piece dtw databases.
		{ EGDB_DTW, "dtw-7-w.bin", 7, true, false, 0x93309a47 },
		{ EGDB_DTW, "dtw-7-b.bin", 7, true, false, 0xbe250495 },

		// 6 piece dtw databases.
		{ EGDB_DTW, "dtw-6.bin", 6, false, false, 0xf629a8a6 },

		// 5 piece dtw databases.
		{ EGDB_DTW, "dtw-5.bin", 5, false, false, 0xfc72714c },

		// 4 piece dtw databases.
		{ EGDB_DTW, "dtw-4.bin", 4, false, false, 0xd0299f2b },

		// 3 piece dtw databases.
		{ EGDB_DTW, "dtw-3.bin", 3, false, false, 0xbe317135 },

		// 2 piece dtw databases.
		{ EGDB_DTW, "dtw-2.bin", 2, false, false, 0xa9c34f3b },

		// 8-piece mtc databases
		{ EGDB_MTC_RUNLEN, "mtc-8-w.bin", 8, true, true, 0x5357878d },
		{ EGDB_MTC_RUNLEN, "mtc-8-b.bin", 8, true, true, 0xa87713d9 },

		// 7-piece mtc databases
		{ EGDB_MTC_RUNLEN, "mtc-7-w.bin", 7, true, true, 0x3acb14e3 },
		{ EGDB_MTC_RUNLEN, "mtc-7-b.bin", 7, true, true, 0x4f3d1544 },

		// 6-piece mtc databases
		{ EGDB_MTC_RUNLEN, "mtc-6.bin", 6, false, true, 0x9815b3c3 },

		// 5-piece mtc databases
		{ EGDB_MTC_RUNLEN, "mtc-5.bin", 5, false, true, 0xc75f3a0a },

		// 8-piece wld databases
		{ EGDB_WLD_TUN_V2, "wld-8-w.bin", 8, true, true, 0xb81e6490 },
		{ EGDB_WLD_TUN_V2, "wld-8-b.bin", 8, true, true, 0x3d0b25e7 },

		// 7-piece wld databases
		{ EGDB_WLD_TUN_V2, "wld-7-w.bin", 7, true, true, 0x88c30869 },
		{ EGDB_WLD_TUN_V2, "wld-7-b.bin", 7, true, true, 0xed601c40 },

		// 6-piece wld databases
		{ EGDB_WLD_TUN_V1, "wld-6.bin", 6, false, true, 0x45f05335 },

		// 5-piece wld databases
		{ EGDB_WLD_TUN_V1, "wld-5.bin", 5, false, true, 0xac6334d2 },

		// 4-piece wld databases
		{ EGDB_WLD_TUN_V1, "wld-4.bin", 4, false, true, 0x76b2e3f5 },

		// 3-piece wld databases
		{ EGDB_WLD_RUNLEN, "wld-3.bin", 3, false, true, 0xde33b1e7 },

		// 2-piece wld databases
		{ EGDB_WLD_RUNLEN, "wld-2.bin", 2, false, true, 0xa5133b3a },

		// end of list
		{ EGDB_NONE, "", 0, false, false, 0 }
	};


	/**
	* Given a directory path, determines what kind of egdb is in it.
	* It does this by looking for a .bin file in the directory, and matching
	* it against a known list of database files.
	* @param db_path a path to the directory that contains the database files.
	* @param egdb_type a pointer that will be populated with one of EGDB_TYPE.
	* @param max_pieces a pointer that will be populated with the max number of
	* pieces in the database.
	* @return 0 if success, 1 if failure.
	*/
	EGDB_API int egdb_identify(const char *db_path, EGDB_TYPE *egdb_type, int *max_pieces)
	{
		int i, j;
		const EGDB_FIND_INFO *db_info;
		std::string s;
		unsigned int crc;

        // Special case: check for our db2.cpr file to identify our database
        s = db_path;
        s += "/db2.cpr";
        FILE *f = fopen(s.c_str(), "rb");
        if (f) {
            fclose(f);
            *egdb_type = EGDB_WLD_RUNLEN;
            *max_pieces = MAXPIECE;
            return 0;
        }

		// find the highest piece count db that exists in the directory.
		for (i = MAXPIECE; i > 1; --i) {
			for (j = 0; (db_info = &egdb_find_table[j])->num_pieces > 0; ++j) {
				if (db_info->num_pieces != i)
					continue;

				// see if this file exists and has the correct crc.
				s = db_path;
				s += "/";
				s += db_info->filename;
				if (fname_crc_calc(s.c_str(), &crc) == 0) {
					if ((db_info->crc == 0) || (crc == db_info->crc)) {
						*egdb_type = db_info->type;
						*max_pieces = i;
						return(0);
					}
				}
			}
		}

		return(1);
	}

}	// namespace egdb_interface