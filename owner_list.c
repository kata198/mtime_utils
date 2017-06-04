/*
 * Copyright (c) 2017 Timothy Savannah under terms of GPLv3
 *
 * owner_list.c - Main for owner_list
 */

#include "mtime_utils.h"

#include "owner_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>



/*typedef struct {
    uid_t pw_uid;
    char *pw_name;

    void *next;
} OwnerInfoListNode;

typedef struct {
    OwnerInfoListNode *first;
} OwnerInfoList;
*/


OwnerInfoList *OwnerInfoList_New(void)
{
    OwnerInfoList *ret;

    ret = malloc( sizeof(OwnerInfoList) );

    ret->first = NULL;

    return ret;
}

void OwnerInfoList_Free(OwnerInfoList *ownerInfoList)
{
    OwnerInfoListNode *cur, *prev;

    cur = ownerInfoList->first;

    while ( cur != NULL )
    {
       free(cur->pw_name);

       prev = cur;
       cur = NODE_NEXT(cur);

       free(prev);
    }

    free(ownerInfoList);
}

OwnerInfoListNode *OwnerInfoList_AddNode(OwnerInfoList *ownerInfoList, uid_t pw_uid, const char *pw_name)
{
    char *nameCopy;
    OwnerInfoListNode *node;

    node = malloc(sizeof(OwnerInfoListNode));

    nameCopy = malloc(strlen(pw_name) + 1);
    strcpy(nameCopy, pw_name);

    node->pw_uid = pw_uid;
    node->pw_name = nameCopy;
    node->next = NULL;

    if ( unlikely( ownerInfoList->first == NULL ) )
    {
        ownerInfoList->first = node;
    }
    else
    {
        OwnerInfoListNode *cur;

        cur = ownerInfoList->first;
        while(cur->next != NULL)
        {
            cur = NODE_NEXT(cur);
        }

        cur->next = node;

    }

    return node;
}

const char *OwnerInfoList_GetName(OwnerInfoList *ownerInfoList, uid_t pw_uid)
{
    OwnerInfoListNode *cur;

    cur = ownerInfoList->first;

    while ( cur != NULL )
    {
        if ( cur->pw_uid == pw_uid )
            return cur->pw_name;

        cur = NODE_NEXT(cur);
    }

    struct passwd *pw;

    pw = getpwuid(pw_uid);

    cur = OwnerInfoList_AddNode(ownerInfoList, pw_uid, pw->pw_name);

    return cur->pw_name;
}


