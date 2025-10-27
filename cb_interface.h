#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "checkers_types.h" // Include all common types and structs
#include "CBconsts.h" // Include for CB_BLACK, CB_MAN, etc.

/* Prevent name mangling of exported dll publics. */
#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif
