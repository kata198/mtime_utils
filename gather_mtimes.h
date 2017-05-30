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
 * NameTime - A struct of provided-filename, and mtime associated.
 *   This is the object that will be sorted.
 */
typedef struct {
    char   *fname;
    time_t  mtime;

} NameTime;

/*
 * ReadNameTimeBuffers - Some "worker" data used for producting the NameTime data.
 *   Created once by initReadNameTimeBuffers,
 *   destroyed with destroyReadNameTimeBuffers.
 *
 *   Destroying this buffer will free data pointed-to by NameTime objects,
 *     so do it only when you are done with the data.
 */
typedef struct {
    FILE *inputStream;
    char *inputStreamBuf;
    size_t inputStreamSize;
    char **lines;

} ReadNameTimeBuffers;


/**
 * initReadNameTimeBuffers - Return created ReadNameTimeBuffers object.
 *    Should be called only once per app.
 */
extern ReadNameTimeBuffers *initReadNameTimeBuffers(void);

/**
 * destroyReadNameTimeBuffers - Destroy the ReadNameTimeBuffers object.
 *    Should be called only after all NameTime data is done being used,
 *      as the buffered data overlaps (not copied)
 */
extern void destroyReadNameTimeBuffers(ReadNameTimeBuffers *buffers);

/**
 * readAndCreateNameTimes - Reads a list of files (separated by newline) from a given stream,
 *   parses and sanitizes ( e.x. removes empty lines).
 *
 *   buffers - Should be the object returned by #initReadNameTimeBuffers
 *
 *   numEntries - A pointer which will be filled with the number of valid entries in return value
 *
 *   stream  - Stream from whence to read data (like stdin)
 */
extern NameTime* readAndCreateNameTimes(ReadNameTimeBuffers *buffers, size_t *numEntries, FILE *stream);

#if defined __USE_XOPEN2K8 || __GLIBC_USE (LIB_EXT2)
#define HAS_MSTREAM
#endif


#endif
