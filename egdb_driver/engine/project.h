#pragma once

#include <cstdint> // For uint64_t

#ifdef _DEBUG
#undef NDEBUG
#else
#ifndef NDEBUG
#define NDEBUG
#endif
#endif

#include <cassert>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define NUMSQUARES 32
#define NUM_BITBOARD_BITS 32
#define MAXPIECE 11
#define ROWSIZE 4
#define RANK0MAX 4

