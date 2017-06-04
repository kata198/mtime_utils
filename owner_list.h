/*
 * Copyright (c) 2017 Timothy Savannah under terms of GPLv3
 *
 * owner_list.h - Header for owner_list.h
 *
 */
#ifndef __OWNER_LIST_H
#define __OWNER_LIST_H


#include <sys/types.h>
#include <pwd.h>

typedef struct {
    uid_t pw_uid;
    char *pw_name;

    void *next;
} OwnerInfoListNode;

#define NODE_NEXT(_info_list) ( (OwnerInfoListNode*)_info_list->next)

typedef struct {
    OwnerInfoListNode *first;
} OwnerInfoList;


#ifdef __INCLUDE_OWNER_LIST_C
#include "owner_list.c"
#endif



#endif
