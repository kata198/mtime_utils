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

#include "gather_mtimes.h"

#define ERROR_ALLOC_MEMORY 12

static const volatile char* APP_NAME = "sort_mtime";

/*
 * printUsage - Prints usage information to stderr
 */
static void printUsage(void)
{
    fputs("Usage: sort_mtime (Options)\n  Takes input of filenames on stdin, sorts based on mtime, and prints to stdout\n\n", stderr);
    fputs("    Options:\n\n", stderr);
    fputs("      -r         Reverse. Show newest on top. Default is newest on bottom.\n\n", stderr);
    fputs("      --help     Print this help message.\n\n", stderr);
    fputs("      --version  Show version information\n\n", stderr);
    fputs("Example:  find . -name '*.gcda' | sort_mtime\n\n", stderr);
}



/**
 * compare_NameStat_mtime - Function called by qsort for comparing two NameStat objects.
 */
int compare_NameStat_mtime(const void *_item1, const void *_item2)
{
    NameStat *item1, *item2;

    item1 = (NameStat *)_item1;
    item2 = (NameStat *)_item2;

    return item1->statBuf.st_mtime - item2->statBuf.st_mtime;
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
        else if ( strcmp("--version", argv[i]) == 0 )
        {
            printVersion(APP_NAME);
            return 0;
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
    ReadNameStatBuffers *buffers;
    NameStat *nameStats = NULL;
    size_t numEntries;
    int i;
    int isReverse;

    /* Parse args.
     *  If return is >= 0, we should exit with that code.
     */
    if ( (i = handleArgs ( argc, (char **)argv, &isReverse ) ) >= 0 )
        return i;

    buffers = initReadNameStatBuffers();
    if ( buffers == NULL )
        return ERROR_ALLOC_MEMORY;

    nameStats = readAndCreateNameStats(buffers, &numEntries, stdin);
    if ( nameStats == NULL )
        goto cleanup_and_exit;

    /*
     * Sort the times. We always sort in the same direction, but
     *   depending on #isReverse we may iterate backwards.
     */
    qsort( nameStats, numEntries, sizeof(NameStat), compare_NameStat_mtime );


    /*
     * Print results in order expected
     */
    if ( !isReverse )
    {
        for(i=0; i < numEntries; i++)
        {
            if( likely(nameStats[i].statBuf.st_mtime != 0) )
                printf("%s\n", nameStats[i].fname);
        }
    }
    else
    {
        for( i=numEntries-1; i >= 0; i--)
        {
            if( likely(nameStats[i].statBuf.st_mtime != 0) )
                printf("%s\n", nameStats[i].fname);
        }
    }

cleanup_and_exit:
    /* Final cleanup */
    if ( nameStats != NULL )
        free(nameStats);
    destroyReadNameStatBuffers(buffers);

    return 0;
}
