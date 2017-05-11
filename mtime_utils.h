/**
 * mtime utils - Copyright (c) 2017 Timothy Savannah All rights reserved
 *   Under terms of GPLv3
 */
#ifndef _MTIME_UTILS_H
#define _MTIME_UTILS_H


#ifdef __GNUC__

  #define ALWAYS_INLINE __attribute__((always_inline))

  #define likely(x)    __builtin_expect(!!(x),1)
  #define unlikely(x)  __builtin_expect(!!(x),0)
  #define __hot __attribute__((hot))

#else

  #define ALWAYS_INLINE
  #define likely(x)   (x)
  #define unlikely(x) (x)
  #define __hot

#endif

extern const volatile char *MTIME_UTILS_VERSION;
extern const volatile char *MTIME_UTILS_COPYRIGHT;

/**
 * printVersion - Print a version string which includes #appName
 *
 * appName - The name of your application
 */
extern void printVersion(const volatile char *appName);

#endif
