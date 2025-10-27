// utility.c
// Replacement for read_text_file using Qt

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "utility.h" // Keep existing prototypes if needed, or update header
#include "checkers_types.h" // For READ_TEXT_FILE_ERROR_TYPE

// *** Original read_text_file function using Windows API is removed ***

// New implementation using Qt
char *read_text_file_qt(const QString &filename, READ_TEXT_FILE_ERROR_TYPE &etype)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Error opening file:" << filename << file.errorString();
        etype = RTF_FILE_ERROR;
        return nullptr;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // Convert QString content to char*
    // NOTE: The caller is responsible for freeing this memory!
    // We need to store the data in a QByteArray first to manage the underlying char array.
    // A simple .toUtf8().data() would point to temporary memory.
    QByteArray utf8Content = content.toUtf8();
    char *buffer = (char *)malloc(utf8Content.size() + 1); // +1 for null terminator

    if (!buffer) {
        qWarning() << "Memory allocation failed when reading file:" << filename;
        etype = RTF_MALLOC_ERROR;
        return nullptr;
    }

    strcpy(buffer, utf8Content.constData());
    etype = RTF_NO_ERROR;
    return buffer;
}


// --- Keep other utility functions below if they exist ---
// Example: CBlog (Needs porting if it uses Windows specifics)
#include <stdio.h>
#include <stdarg.h>

void CBlog(const char *fmt, ...)
{
    // For now, just print to qDebug. A proper logging solution can be added later.
    va_list args;
    va_start(args, fmt);
    QString message = QString::vasprintf(fmt, args);
    va_end(args);
    qDebug() << message;

    // Original file logging logic would go here, adapted using QFile/QTextStream
    // FILE *fp;
    // fp = fopen("CBlog.txt", "a");
    // if (fp != NULL) {
    //     vfprintf(fp, fmt, args);
    //     fclose(fp);
    // }
}

void cblog(const char *fmt, ...)
{
     // Simple alias or could have different behavior
    va_list args;
    va_start(args, fmt);
    QString message = QString::vasprintf(fmt, args);
    va_end(args);
    qDebug() << message; // Redirect to qDebug for now
    va_end(args);
}


// --- strncpy_terminated ---
// Ensure null termination for strncpy
char *strncpy_terminated(char *dest, const char *src, size_t n) {
    strncpy(dest, src, n - 1);
    dest[n - 1] = '\0'; // Ensure null termination
    return dest;
}

//--- Add other utility functions below and port them as needed ---

