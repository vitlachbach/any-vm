/* Copyright (c) 2017 Nguyen Viet Giang. All rights reserved. */
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <any/platform.h>
#include <any/list.h>

typedef struct atask_t {
    LPVOID fiber;
    alist_node_t node;
} atask_t;