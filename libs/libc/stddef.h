#ifndef STDDEF_H
#define STDDEF_H
#include <stdint.h>
#define NULL 0
typedef uint64_t size_t;

#ifdef __STRICT_ANSI__
#define wfinline __attribute__((unused))
#else
#define wfinline __attribute__((always_inline))
#endif
typedef decltype(nullptr) nullptr_t;
#endif
