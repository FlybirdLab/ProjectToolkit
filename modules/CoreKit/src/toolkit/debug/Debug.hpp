// Created by cubevlmu on 2025/8/5.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

namespace toolkit {

    typedef void(*stack_trace_func)(const char*);
    void getStackTrace(const stack_trace_func);

}