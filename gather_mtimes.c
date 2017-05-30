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
    size_t numLines;

    for( numLines=0; *buf != '\0'; buf++ )
    {
        if ( *buf == '\n' )
            numLines += 1;
    }
    
    return numLines;
}

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
 */
static char** splitLines(char *buf, size_t *numLines)
{
    char **ret;
    int i;
    size_t _numLines;
    char *tmp;

    _numLines = getNumLines(buf);

    ret = malloc( sizeof(char*) * (_numLines) );

    ret[0] = buf;

    for( i=1; i < _numLines; i++)
    {
        ret[i] = strchr(ret[i-1] + 1, '\n');

        *ret[i] = '\0';
        ret[i] += 1;

        while( *ret[i] == '\n' )
        {
            *ret[i] = '\0';
            ret[i] += 1;
            _numLines -= 1;
            if ( unlikely( ret[i] == '\0' ) )
            {
                break;
            }
        } 
    }

   tmp = ret[ _numLines - 1];

   tmp[ strlen(tmp) - 1 ] = '\0';
   *numLines = _numLines;

    return ret;
}

/**
 * getNameTimes - Take in a list of names (and a size),
 *   query the mtimes for each, and return a list of NameTime objects
 *   intended for sorting.
 *
 *   If a file cannot be lstat'd, a message will be printed to stderr,
 *   and the mtime will be set to 0. These items should not be printed.
 */
static NameTime* getNameTimes( char **names, size_t numLines )
{
    NameTime *ret;
    struct stat stat_buf;
    int i;

    int statRet;

    ret = malloc( sizeof(NameTime) * (numLines + 1 ) );

    for ( i=0; i < numLines; i++ )
    {

        ret[i].fname = names[i];
        statRet = lstat(names[i], &stat_buf);
        if ( likely( statRet >= 0 ) )
        {
            ret[i].mtime = stat_buf.st_mtime;
        }
        else
        {
            fprintf(stderr, "Err: Cannot stat file: %s\n", ret[i].fname);
            ret[i].mtime = 0;
        }
    }

    return ret;

}

ReadNameTimeBuffers *initReadNameTimeBuffers(void)
{
    ReadNameTimeBuffers *buffers;

    buffers = malloc( sizeof(ReadNameTimeBuffers) );

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

void destroyReadNameTimeBuffers(ReadNameTimeBuffers *buffers)
{
    free(buffers->lines);

    fclose(buffers->inputStream);

    free(buffers->inputStreamBuf);
    free(buffers);
}


NameTime* readAndCreateNameTimes(ReadNameTimeBuffers *buffers, size_t *numEntries, FILE *stream)
{
    size_t numBytesRead;
    char *buf;

    char **lines;
    NameTime* nameTimes;
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
     * Stat the files, return a NameTimes array, with non-zero mtime for
     *  files that could be stat'd
     */
    nameTimes = getNameTimes(lines, *numEntries);

    return nameTimes;

}
