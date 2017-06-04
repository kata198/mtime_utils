/*
 * Copyright (c) 2017 Timothy Savannah under terms of GPLv3
 *
 * group_list.h - Header for group_list.h
 *
 */
#ifndef __GROUP_LIST_H
#define __GROUP_LIST_H


#include <sys/types.h>
#include <grp.h>

typedef struct {
    gid_t pw_gid;
    char *gr_name;

    void *next;
} GroupInfoListNode;

#define NODE_NEXT(_info_list) ( (GroupInfoListNode*)_info_list->next)

typedef struct {
    GroupInfoListNode *first;
} GroupInfoList;


#ifdef __INCLUDE_GROUP_LIST_C
#include "group_list.c"
#endif



#endif
