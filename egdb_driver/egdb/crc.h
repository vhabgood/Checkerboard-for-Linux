/*
* Copyright (C) 2017-2021 by E. Gilbert
*
* This file is part of the egdb_intl library.
*
* This library is free software; you can redistribute it and/or modify it
* under the terms of the Boost Software License, Version 1.0.
*/

#ifndef CRC_H
#define CRC_H

#include <cstdio>


namespace egdb_interface {

	unsigned int crc_calc(char const *buf, int len);
	unsigned int file_crc_calc(FILE *fp, int *abort);
	int fname_crc_calc(char const *name, unsigned int *crc);

}	// namespace egdb_interface

#endif
