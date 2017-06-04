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
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "mtime_utils.h"

#include "gather_mtimes.h"

#define ERROR_ALLOC_MEMORY 12

static const volatile char* APP_NAME = "get_mtime";

/*
 * printUsage - Prints usage information to stderr
 */
static void printUsage(void)
{
    fputs("Usage: get_mtime (Options)\n  Takes input of filenames on stdin, and prints the name,\n     followed by mtime to stdout\n\n", stderr);
    fputs("    Options:\n\n", stderr);
    fputs("      -e  --epoch   Print epoch time.\n\n", stderr);
    fputs("      --format=X    Print time using strformat string, 'X'. See man strftime\n\n", stderr);
    fputs("      --help     Print this help message.\n\n", stderr);
    fputs("      --version  Show version information\n\n", stderr);
    fputs("Default output format is ctime.\nIf you want to easily sort the output, pipe names to sort_mtime, then pipe output to get_mtime\n\n", stderr);
}


/**
 * handleArgs - Handle args on commandline.
 *
 *   Sets isEpoch to 1 if -r was specified, otherwise 0.
 *
 *
 * If return is >= 0, the program should exit with that code.
 */
static inline int handleArgs(int argc, char **argv, int *isEpoch, char **customFormat)
{
    int i;

    *isEpoch = 0;

    for( i=1; i < argc; i++ )
    {
        if ( strcmp("--help", argv[i]) == 0 )
        {
            printUsage();
            return 0;
        }
        else if ( strcmp("-e", argv[i]) == 0 || strcmp("--epoch", argv[i]) == 0 )
        {
            if ( *isEpoch == 1 )
            {
                fputs("-e or --epoch provided more than once.\n", stderr);
                return 1;
            }
            *isEpoch = 1;
        }
        else if( strstr(argv[i], "--format=") == argv[i] )
        {
            customFormat[0] = argv[i] + 9;
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
    int isEpoch;
    char *customFormat = NULL;

    /* Parse args.
     *  If return is >= 0, we should exit with that code.
     */
    if ( (i = handleArgs ( argc, (char **)argv, &isEpoch, &customFormat ) ) >= 0 )
        return i;

    buffers = initReadNameStatBuffers();
    if ( buffers == NULL )
        return ERROR_ALLOC_MEMORY;

    nameStats = readAndCreateNameStats(buffers, &numEntries, stdin);
    if ( !nameStats )
        goto cleanup_and_exit;

    if ( !isEpoch )
    {
        char *timeBuff = malloc(64);
        if ( customFormat == NULL )
        {
            for(i=0; i < numEntries; i++)
            {
                if ( likely(nameStats[i].statBuf.st_mtime != 0) )
                {
                    ctime_r(&nameStats[i].statBuf.st_mtime, timeBuff);
                    
                    printf("%s\t%s", nameStats[i].fname, timeBuff);
                }
            }
        }
        else
        {
            struct tm *tmpTm;
            for(i=0; i < numEntries; i++)
            {
                if ( likely(nameStats[i].statBuf.st_mtime != 0) )
                {
                    tmpTm = localtime(&nameStats[i].statBuf.st_mtime);
                    strftime(timeBuff, 64, customFormat, tmpTm);
                    
                    printf("%s\t%s\n", nameStats[i].fname, timeBuff);
                }
            }


        }
        free(timeBuff);
    }
    else
    {
        for(i=0; i < numEntries; i++)
        {
            if ( likely(nameStats[i].statBuf.st_mtime != 0) )
            {
                printf("%s\t%ld\n", nameStats[i].fname, nameStats[i].statBuf.st_mtime);
            }
        }
    }

cleanup_and_exit:

    /* Final cleanup */
    if ( nameStats != NULL )
        free(nameStats);
    destroyReadNameStatBuffers(buffers);

    return 0;
}
