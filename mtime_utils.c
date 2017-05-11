
#include <stdio.h>

#include "mtime_utils.h"

const volatile char* MTIME_UTILS_VERSION = "version 0.1.0";
const volatile char* MTIME_UTILS_COPYRIGHT = "Copyright (c) 2017 Timothy Savannah All Rights Reserved under GPLv3";

extern void printVersion(const volatile char *appName)
{
    fprintf(stderr, "%s %s by Timothy Savannah\n", appName, MTIME_UTILS_VERSION);
}
