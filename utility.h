#ifndef UTILITY_H
#define UTILITY_H

#include <QString> // Include necessary Qt header
#include "checkers_types.h" // For READ_TEXT_FILE_ERROR_TYPE

// Function Prototypes

// Replaces original read_text_file. Caller MUST free the returned char* buffer.
char *read_text_file_qt(const QString &filename, READ_TEXT_FILE_ERROR_TYPE &etype);

// Logging functions (simple qDebug wrappers for now)
void CBlog(const char *fmt, ...);
void cblog(const char *fmt, ...);

// Safe string copy
char *strncpy_terminated(char *dest, const char *src, size_t n);

// Declare other utility functions if needed

#endif // UTILITY_H

