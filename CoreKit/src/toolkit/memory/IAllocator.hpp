// Created by cubevlmu on 2025/8/4.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/globals.hpp>

namespace toolkit {

    class IAllocator
    {
    public:
        virtual ~IAllocator() = default;

        virtual void* alloc(PtrSize size, PtrSize align) = 0;
        virtual void* realloc(void* ptr, PtrSize newSize, PtrSize oldSize, PtrSize align) = 0;
        virtual void free(void* ptr) = 0;

        virtual IAllocator* getParent() const {
            return nullptr;
        }

#if TK_DEBUG
        TK_FORCE_INLINE const char* getDebugName() {
            return m_debugName ? m_debugName : "nonamed";
        }
#endif
    private:
#if TK_DEBUG
        const char* m_debugName = nullptr;
#endif
    };
}