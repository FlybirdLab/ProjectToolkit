/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */
// Created by cubevlmu on 2025/8/5.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#define TK_USE_RPMALLOC 1
#define TK_TRACY_ENABLE 0
#define TK_CRASH_LOG    1
#define TK_HEADLESS     0

#define TK_WINDOW_KIT   0

#ifndef DICTIONARY_DEFAULT_CAPACITY
#if TK_WINDOWS || TK_MACOS || TK_LINUX
#define DICTIONARY_DEFAULT_CAPACITY 256
#else
#define DICTIONARY_DEFAULT_CAPACITY 64
#endif
#endif

// Default slack space divider for the dictionaries.
#define DICTIONARY_DEFAULT_SLACK_SCALE 3
// Function for dictionary that tells how change hash index during iteration (size param is a buckets table size).
#define DICTIONARY_PROB_FUNC(size, numChecks) (numChecks)
//#define DICTIONARY_PROB_FUNC(size, numChecks) (1)
