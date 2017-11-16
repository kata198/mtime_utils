/*
 * Copyright (c) 2017 Timothy Savannah under terms of GPLv3
 *
 * gather_mtimes.c - Gathers filenames and mtimes from stdin
 */

#include <features.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "mtime_utils.h"

#include "gather_mtimes.h"

/*
 * BUF_SIZE - Number of bytes we read from stdin in a single block.
 */
#define BUF_SIZE 65535

/* 
 * getNumLines - Count the number of newline characters in buffer
 */
static inline size_t getNumLines(char *buf)
{
    size_t numLines = 0;

    for( ; *buf != '\0'; buf++ )
    {
        if ( *buf == '\n' )
            numLines += 1;
    }
    
    return numLines;
}

static char EMPTY_STR[] = { 0 };

/*
 * splitLines - Take a buffer, and return a char** with each
 *   pointer pointing at the start of each line.
 *
 *   Empty lines are ignored.
 *
 *   All newline characters are overwritten with '\0'
 *
 *   *numLines will contain the number of non-empty lines
 *     (and matches the size of return)
 *
 *    NOTE - This WILL overwrite data in $buf, and $buf must remain
 *             allocated as the return points within that buffer
 */
static char** splitLines(char *buf, size_t *numLines)
{
    char **ret; /* Our array to return */
    size_t _numLines;

    int i;
    char *lastEntry;
    char *lastEntryEnd;


    _numLines = getNumLines(buf);


    if ( unlikely( _numLines == 0 ) )
    {
        /* This cannot occur when using the readAndCreateNameStats method,
             as it will append a tailing newline if one is not present.

             But prepare and handle other circumstances anyway.
        */
        ret = malloc( sizeof(char*) );
        ret[0] = EMPTY_STR;

        *numLines = 0;
        return ret;
    }

    ret = malloc( sizeof(char*) * (_numLines) );

    /* Point first line to start of the buff */
    ret[0] = buf;

    /* If we start with newline characters, keep moving the first
        entry forward by 1 char until we hit a non-newline (And thus start of data)
    */
    while ( *ret[0] == '\n' )
    {
        ret[0] += 1;
        _numLines -= 1;
    }

    for( i=1; i < _numLines; i++)
    {
        /* For all the rest of the lines:
             1. Point to first character after first newline character found at previous
                  line pointer.
             2. Zero out that newline character (thus sealing the previous line)
        */
        ret[i] = strchr(ret[i-1] + 1, '\n');

        *ret[i] = '\0';
        ret[i] += 1;

        /* Check if our new line begins with a newline character */
        while( *ret[i] == '\n' )
        {
            /* If so, move the pointer forward and subtract expected number of lines */
            *ret[i] = '\0';
            ret[i] += 1;
            _numLines -= 1;
        } 
    }

    if ( _numLines == 0 )
    {
        /* If all we had was newlines, zero out the first slot */
        ret[0][0] = '\0';
    }
    else
    {
        /* Otherwise, since our loop above "seals" the previous line each time,
            depending on tailing newlines etc. we may need to seal the current
            final line if it contains a newline, replacing it with a 0 byte
        */
        lastEntry = ret[ _numLines - 1];

        /*lastEntryEnd = &lastEntry[ strlen(lastEntry) - 1];
*/
        lastEntryEnd = strchr(lastEntry, '\n');

        /*if ( *lastEntryEnd == '\n' )*/
        if ( lastEntryEnd != NULL )
        {
            *lastEntryEnd = '\0';
        }
    }

    *numLines = _numLines;

    return ret;
}

/**
 * getNameStats - Take in a list of names (and a size),
 *   query the mtimes for each, and return a list of NameStat objects
 *   intended for sorting.
 *
 *   If a file cannot be lstat'd, a message will be printed to stderr,
 *   and the mtime will be set to 0. These items should not be printed.
 */
static NameStat* getNameStats( char **names, size_t numLines )
{
    NameStat *ret;
    int i;

    int statRet;

    ret = malloc( sizeof(NameStat) * (numLines + 1 ) );

    for ( i=0; i < numLines; i++ )
    {

        ret[i].fname = names[i];
        statRet = lstat(names[i], &ret[i].statBuf);
        if ( unlikely( statRet < 0 ) )
        {
            fprintf(stderr, "Err: Cannot stat file: %s\n", ret[i].fname);
            
            memset(&ret[i].statBuf, 0x0, sizeof(struct stat));
        }
    }

    return ret;

}

ReadNameStatBuffers *initReadNameStatBuffers(void)
{
    ReadNameStatBuffers *buffers;

    buffers = malloc( sizeof(ReadNameStatBuffers) );

    #if defined(HAS_MSTREAM)
      /* Open an automatically-expanding memstream, into which we will write eveything on stdin */
      buffers->inputStream = open_memstream( &(buffers->inputStreamBuf), &(buffers->inputStreamSize));
      if ( unlikely( !(buffers->inputStream) ) )
      {
          fputs("Failed to allocate memory stream.\n", stderr);
          return NULL;
      }
      /* fflush here to set inputStreamBuf to allocated buffer, and 0 inputStreamSize */
      fflush(buffers->inputStream);
    #else
      /* Otherwise, use a tmpfile */
      buffers->inputStream = tmpfile();
      if ( unlikely( !(buffers->inputStream) ) )
      {
        fputs("Failed to create temp file.\n", stderr);
        return NULL;
      }
      buffers->inputStreamBuf = NULL;
      buffers->inputStreamSize = 0;
    #endif

    buffers->lines = NULL;
    
    return buffers;
}

void destroyReadNameStatBuffers(ReadNameStatBuffers *buffers)
{
    free(buffers->lines);

    fclose(buffers->inputStream);

    free(buffers->inputStreamBuf);
    free(buffers);
}


NameStat* readAndCreateNameStats(ReadNameStatBuffers *buffers, size_t *numEntries, FILE *stream)
{
    size_t numBytesRead;
    char *buf;

    char **lines;
    NameStat* nameTimes;
    FILE *inputStream;
    char *inputStreamBuf;


    numBytesRead = 0;
    buf = malloc( BUF_SIZE );

    inputStream = buffers->inputStream;

    /* 
     * Read from stream into a temp buff, then write the same
     *   into the memstream
     */
    do {
        numBytesRead = fread(buf, 1, BUF_SIZE, stream);
        fwrite(buf, 1, numBytesRead, inputStream);
        fflush(inputStream);
    }while ( ! feof(stream) );

    free(buf);

    /* If we did not read any data, just exit */
    if ( unlikely( numBytesRead == 0 ) )
        return NULL;

    #if !defined(HAS_MSTREAM)
      /* If we don't have mstream support, then we use a tmpfile, and must manually
       *   set our sizes and read our data
       */
      struct stat statBuf;
      fstat( fileno(inputStream), &statBuf);

      if ( buffers->inputStreamBuf == NULL )
      {
          /* First read on this buffer */
          buffers->inputStreamSize = statBuf.st_size;
          buffers->inputStreamBuf = malloc( statBuf.st_size + 1 );

          rewind(inputStream);
          read( fileno(inputStream), buffers->inputStreamBuf, buffers->inputStreamSize );
      }
      else
      {
        /* This function has been called before. Copy buffers and append */
        size_t oldSize = buffers->inputStreamSize;

        buffers->inputStreamSize = statBuf.st_size;
        buffers->inputStreamBuf = realloc(buffers->inputStreamBuf, statBuf.st_size + 1);

        fseek( inputStream, oldSize, SEEK_SET );
        read( fileno(inputStream), &buffers->inputStreamBuf[oldSize], buffers->inputStreamSize - oldSize );
      }
      buffers->inputStreamBuf[buffers->inputStreamSize] = '\0';
    #endif

    inputStreamBuf = buffers->inputStreamBuf;

    /* Treat just newline same as no data */
    if ( unlikely( numBytesRead == 1 && *inputStreamBuf == '\n' ) )
        return NULL;

    /* If we did not have a final newline, append one. */
    if( inputStreamBuf[numBytesRead - 1] != '\n' )
    {
        fputc('\n', inputStream);
        fflush(inputStream);
    }

    /*
     * -- Process Data
     */

    /*
     * Split up the input stream into non-empty lines
     */
    lines = splitLines(inputStreamBuf, numEntries);

    buffers->lines = lines;

    /*
     * Stat the files, return a NameStats array, with non-zero mtime for
     *  files that could be stat'd
     */
    nameTimes = getNameStats(lines, *numEntries);

    return nameTimes;

}
