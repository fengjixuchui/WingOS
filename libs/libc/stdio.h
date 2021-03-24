#ifndef STDIO_H
#define STDIO_H
#ifdef __cpluplus
extern "C"
{
#endif
#include <plug/system_plug.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

    int printf(const char *format, ...);
    int vsprintf(char *buffer, const char *format, va_list vlist);
    int sprintf(char *buffer, const char *format, ...);

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#define EOF 0x0E0F

    // TEMPORARY FIX FOR LATER

    struct FILE
    {
        int file_element;
    };

    typedef struct FILE FILE;

    extern FILE *stdin;
    extern FILE *stdout;
    extern FILE *stderr;

    FILE *fopen(const char *pathname, const char *mode);
    int fclose(FILE *stream);

    size_t fread(void *ptr, size_t size, size_t count, FILE *stream);
    int fgetc(FILE *stream);

    int ungetc(int c, FILE *stream); // not implemented

    int fseek(FILE *stream, long offset, int whence);
    long ftell(FILE *stream);

    int feof(FILE *stream);
    int ferror(FILE *stream);

#ifdef __cpluplus
}
#endif
#endif
