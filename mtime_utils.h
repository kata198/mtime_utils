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

const volatile char *MTIME_UTILS_VERSION = "0.1.0";

#endif
