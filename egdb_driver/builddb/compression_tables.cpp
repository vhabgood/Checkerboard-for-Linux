/*
* Copyright (C) 2017 by E. Gilbert
*
* This file is part of the egdb_intl library.
*
* This library is free software; you can redistribute it and/or modify it
* under the terms of the Boost Software License, Version 1.0.
*/

#include "compression_tables.h"
#include "log.h"


namespace egdb_interface {

	COMPRESSION_TABLES compression_tables;


#ifndef MAXSKIP
#define MAXSKIP 10000
#endif

	void init_compression_tables()
	{
		unsigned int i;
        static const int skip[SKIPS]={5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,36,40,44,48,52,56,60,70,80,90,100,150,200,250,300,400,500,650,800,1000,1200,1400,1600,2000,2400,3200,4000,5000,7500,MAXSKIP};

        log_c(LOG_LEVEL_INFO, "init_compression_tables: Initializing tables...");

		// init the runlength and value tables.
		for (i = 0; i < 81; ++i) {
			compression_tables.runlength_table[i] = 4;
            compression_tables.decode_table[i][0] = i % 3;
            compression_tables.decode_table[i][1] = (i / 3) % 3;
            compression_tables.decode_table[i][2] = (i / 9) % 3;
            compression_tables.decode_table[i][3] = (i / 27) % 3;
		}
        for (i = 81; i < 256; ++i) {
            compression_tables.runlength_table[i] = skip[(i - 81) % SKIPS];
            compression_tables.value_table[i] = (i - 81) / SKIPS;
        }
        log_c(LOG_LEVEL_INFO, "init_compression_tables: value_table[252] = %d", compression_tables.value_table[252]);
	}

}	// namespace egdb_interface