/*
 * Copyright (c) 2017 Timothy Savannah under terms of GPLv3
 *
 * get_group.c - Gets filename group
 */

#include <features.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>

#include "mtime_utils.h"

#include "gather_mtimes.h"

#define __INCLUDE_GROUP_LIST_C
#include "group_list.h"

#define ERROR_ALLOC_MEMORY 12

static const volatile char* APP_NAME = "get_group";

/*
 * printUsage - Prints usage information to stderr
 */
static void printUsage(void)
{
    fprintf(stderr, "Usage: %s (Options)\n  Takes input of filenames on stdin, and prints\n", APP_NAME);
    fputs("   the 'filename<TAB>group name<TAB>group gid' to stdout.\n\n", stderr);
    fputs("    Options:\n\n", stderr);
    fputs("      --help     Print this help message.\n\n", stderr);
    fputs("      --version  Show version information\n\n", stderr);
}


/**
 * handleArgs - Handle args on commandline.
 *
 *   Sets isEpoch to 1 if -r was specified, otherwise 0.
 *
 *
 * If return is >= 0, the program should exit with that code.
 */
static inline int handleArgs(int argc, char **argv)
{
    int i;

    for( i=1; i < argc; i++ )
    {
        if ( strcmp("--help", argv[i]) == 0 )
        {
            printUsage();
            return 0;
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
 * Ya, I heard you like main's dog, so I put a __start in your main now you can
 *   start your program while your program's startin, dog
 */
int main(int argc, char* argv[])
{
    ReadNameStatBuffers *buffers;
    NameStat *nameStats = NULL;
    GroupInfoList *groupInfoList;
    size_t numEntries;
    int i;

    /* Parse args.
     *  If return is >= 0, we should exit with that code.
     */
    if ( (i = handleArgs ( argc, (char **)argv) ) >= 0 )
        return i;

    buffers = initReadNameStatBuffers();
    if ( buffers == NULL )
        return ERROR_ALLOC_MEMORY;

    groupInfoList = GroupInfoList_New();

    nameStats = readAndCreateNameStats(buffers, &numEntries, stdin);
    if ( !nameStats )
        goto cleanup_and_exit;

    for(i=0; i < numEntries; i++ )
    {
        if ( likely(nameStats[i].statBuf.st_mtime != 0) )
        {
            printf("%s\t%s\t%d\n", nameStats[i].fname, GroupInfoList_GetName(groupInfoList, nameStats[i].statBuf.st_gid), nameStats[i].statBuf.st_gid);

        }
    }

cleanup_and_exit:

    /* Final cleanup */
    if ( nameStats != NULL )
        free(nameStats);

    GroupInfoList_Free(groupInfoList);
    destroyReadNameStatBuffers(buffers);

    return 0;
}
