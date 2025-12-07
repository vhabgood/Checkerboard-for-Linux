#pragma once

// from dtw\dtw.h
#define MAX_DTW_PLIES 255


namespace egdb_interface {

	typedef struct {
		/* a file handle for the database file. */
		FILE_HANDLE file;

		/* true if this db contains positions with n pieces, where n is <= num_pieces.
		* if false, it only contains positions with exactly num_pieces.
		*/
		bool contains_le_pieces;

		/* if true, this database contains entries for one side only. */
		bool dtw_w_only;

		/* size of the file in bytes. */
		uint64_t file_size;

		/* a memory mapped handle to the file. For Windows only. */
		MEM_HANDLE map_handle;

		/* a pointer to the database in memory. */
		uint8_t *db;
	} DTW_DB_INFO;

	typedef struct {
		/* an array of db handles, one for each number of pieces. */
		DTW_DB_INFO *db_info;
	} DTW_DRIVER_INFO;
}