/*
 * Copyright (c) 2017 Timothy Savannah under terms of GPLv3
 *
 * group_list.c - Main for group_list
 */

#include "mtime_utils.h"

#include "group_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <grp.h>



/*typedef struct {
    gid_t pw_gid;
    char *gr_name;

    void *next;
} GroupInfoListNode;

typedef struct {
    GroupInfoListNode *first;
} GroupInfoList;
*/


GroupInfoList *GroupInfoList_New(void)
{
    GroupInfoList *ret;

    ret = malloc( sizeof(GroupInfoList) );

    ret->first = NULL;

    return ret;
}

void GroupInfoList_Free(GroupInfoList *groupInfoList)
{
    GroupInfoListNode *cur, *prev;

    cur = groupInfoList->first;

    while ( cur != NULL )
    {
       free(cur->gr_name);

       prev = cur;
       cur = NODE_NEXT(cur);

       free(prev);
    }

    free(groupInfoList);
}

GroupInfoListNode *GroupInfoList_AddNode(GroupInfoList *groupInfoList, gid_t pw_gid, const char *gr_name)
{
    char *nameCopy;
    GroupInfoListNode *node;

    node = malloc(sizeof(GroupInfoListNode));

    nameCopy = malloc(strlen(gr_name) + 1);
    strcpy(nameCopy, gr_name);

    node->pw_gid = pw_gid;
    node->gr_name = nameCopy;
    node->next = NULL;

    if ( unlikely( groupInfoList->first == NULL ) )
    {
        groupInfoList->first = node;
    }
    else
    {
        GroupInfoListNode *cur;

        cur = groupInfoList->first;
        while(cur->next != NULL)
        {
            cur = NODE_NEXT(cur);
        }

        cur->next = node;

    }

    return node;
}

const char *GroupInfoList_GetName(GroupInfoList *groupInfoList, gid_t pw_gid)
{
    GroupInfoListNode *cur;

    cur = groupInfoList->first;

    while ( cur != NULL )
    {
        if ( cur->pw_gid == pw_gid )
            return cur->gr_name;

        cur = NODE_NEXT(cur);
    }

    struct group *gr;

    gr = getgrgid(pw_gid);

    cur = GroupInfoList_AddNode(groupInfoList, pw_gid, gr->gr_name);

    return cur->gr_name;
}


