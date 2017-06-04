/**
 * gather_mtimes.h - Part of mtime_utils
 *
 *   Copyright (c) 2017 Timothy Savannah all rights reserved
 *     Licensed under terms of the GNU General Purpose License (GPL) Version 3
 */

#ifndef _GATHER_MTIMES_H
#define _GATHER_MTIMES_H

#include <stdio.h>
#include <sys/types.h>

/* 
 * NameStat - A struct of provided-filename, and mtime associated.
 *   This is the object that will be sorted.
 */
typedef struct {
    char        *fname;
    struct stat statBuf;

} NameStat;

/*
 * ReadNameStatBuffers - Some "worker" data used for producting the NameStat data.
 *   Created once by initReadNameStatBuffers,
 *   destroyed with destroyReadNameStatBuffers.
 *
 *   Destroying this buffer will free data pointed-to by NameStat objects,
 *     so do it only when you are done with the data.
 */
typedef struct {
    FILE *inputStream;
    char *inputStreamBuf;
    size_t inputStreamSize;
    char **lines;

} ReadNameStatBuffers;


/**
 * initReadNameStatBuffers - Return created ReadNameStatBuffers object.
 *    Should be called only once per app.
 */
extern ReadNameStatBuffers *initReadNameStatBuffers(void);

/**
 * destroyReadNameStatBuffers - Destroy the ReadNameStatBuffers object.
 *    Should be called only after all NameStat data is done being used,
 *      as the buffered data overlaps (not copied)
 */
extern void destroyReadNameStatBuffers(ReadNameStatBuffers *buffers);

/**
 * readAndCreateNameStats - Reads a list of files (separated by newline) from a given stream,
 *   parses and sanitizes ( e.x. removes empty lines).
 *
 *   buffers - Should be the object returned by #initReadNameStatBuffers
 *
 *   numEntries - A pointer which will be filled with the number of valid entries in return value
 *
 *   stream  - Stream from whence to read data (like stdin)
 */
extern NameStat* readAndCreateNameStats(ReadNameStatBuffers *buffers, size_t *numEntries, FILE *stream);

#if defined __USE_XOPEN2K8 || __GLIBC_USE (LIB_EXT2)
#define HAS_MSTREAM
#endif


#endif
