/*
 * Copyright (c) 2017 Timothy Savannah under terms of GPLv3
 *
 * sort_mtime.c - Sorts list of files by mtime
 */

#include <features.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "mtime_utils.h"

/*
 * BUF_SIZE - Number of bytes we read from stdin in a single block.
 */
#define BUF_SIZE 65535

/* 
 * NameTime - A struct of provided-filename, and mtime associated.
 *   This is the object that will be sorted.
 */
typedef struct {
    char   *fname;
    time_t  mtime;
} NameTime;


/*
 * printUsage - Prints usage information to stderr
 */
static void printUsage(void)
{
    fputs("Usage: sort_mtime (Options)\n  Takes input of filenames on stdin and sorts to stdout based on mtime\n\n", stderr);
    fputs("    Options:\n\n", stderr);
    fputs("      -r      Reverse. Show newest on top. Default is newest on bottom.\n\n", stderr);
    fputs("Example:  find . -name '*.gcda' | sort_mtime\n\n", stderr);
}


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
                break;
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
        if ( statRet >= 0 )
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

/**
 * compare_NameTime - Function called by qsort for comparing two NameTime objects.
 */
int compare_NameTime(const void *_item1, const void *_item2)
{
    NameTime *item1, *item2;

    item1 = (NameTime *)_item1;
    item2 = (NameTime *)_item2;

    return item1->mtime - item2->mtime;
}

/**
 * handleArgs - Handle args on commandline.
 *
 *   Sets isReverse to 1 if -r was specified, otherwise 0.
 *
 *
 * If return is >= 0, the program should exit with that code.
 */
static inline int handleArgs(int argc, char **argv, int *isReverse)
{
    int i;

    *isReverse = 0;

    for( i=1; i < argc; i++ )
    {
        if ( strcmp("--help", argv[i]) == 0 )
        {
            printUsage();
            return 0;
        }

        else if ( strcmp("-r", argv[i]) == 0 )
        {
            if ( *isReverse == 1 )
            {
                fputs("-r provided more than once.\n", stderr);
                return 1;
            }
            *isReverse = 1;
        }
        else
        {
            fprintf(stderr, "Unknown argument: %s\n\n", argv[i]);
            printUsage();
            return 1;
        }
    }

    return -1;
}

/**
 * Ya main' dog
 */
int main(int argc, char* argv[])
{
    int i;
    int isReverse;
    FILE *inputStream;
    char *inputStreamBuf;
    size_t inputStreamSize;


    /* Parse args.
     *  If return is >= 0, we should exit with that code.
     */
    if ( (i = handleArgs ( argc, (char **)argv, &isReverse ) ) >= 0 )
        return i;


    /* Open an automatically-expanding memstream, into which we will write eveything on stdin */
    inputStream = open_memstream(&inputStreamBuf, &inputStreamSize);
    if ( !inputStream )
    {
        fputs("Failed to allocate memory straem.\n", stderr);
        return 2;
    }

    /* 
     * Read from stdin into a temp buff, then write the same
     *   into the memstream
     */
    size_t numBytesRead = 0;
    char *buf = malloc(BUF_SIZE);
    do {
        numBytesRead = fread(buf, 1, BUF_SIZE, stdin);
        fwrite(buf, 1, numBytesRead, inputStream);
        fflush(inputStream);
    }while ( ! feof(stdin) );

    free(buf);

    /* If we did not read any data, just exit */
    if ( numBytesRead == 0 || (numBytesRead == 1 && *inputStreamBuf == '\n') )
        return 0;

    /* If we did not have a final newline, append one. */
    if( inputStreamBuf[numBytesRead - 1] != '\n' )
    {
        fputc('\n', inputStream);
        fflush(inputStream);
    }

    /*
     * -- Process Data
     */
    size_t numLines;
    char **lines;
    NameTime* nameTimes;

    /*
     * Split up the input stream into non-empty lines
     */
    lines = splitLines(inputStreamBuf, &numLines);

    /*
     * Stat the files, return a NameTimes array, with non-zero mtime for
     *  files that could be stat'd
     */
    nameTimes = getNameTimes(lines, numLines);

    /*
     * Sort the times. We always sort in the same direction, but
     *   depending on #isReverse we may iterate backwards.
     */
    qsort( nameTimes, numLines, sizeof(NameTime), compare_NameTime );


    /*
     * Print results in order expected
     */
    if ( !isReverse )
    {
        for(i=0; i < numLines; i++)
        {
            if( likely(nameTimes[i].mtime != 0) )
                printf("%s\n", nameTimes[i].fname);
        }
    }
    else
    {
        for( i=numLines-1; i >= 0; i--)
        {
            if( likely(nameTimes[i].mtime != 0) )
                printf("%s\n", nameTimes[i].fname);
        }
    }

    /* Final cleanup */
    fclose(inputStream);
    free(lines);
    free(nameTimes);
    free(inputStreamBuf);

    return 0;
}
