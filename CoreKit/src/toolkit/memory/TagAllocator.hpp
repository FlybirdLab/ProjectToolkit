// Created by cubevlmu on 2025/8/5.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/memory/Memory.hpp>

namespace toolkit {

    /// Effiective allocator for group allocating and relocate allocator to control allocate logic
    class TagAllocator final : public AllocatorBase
    {
    public:
        TagAllocator(IAllocator allocator, const char* tag_name);
        ~TagAllocator() = default;

        void* alloc(psize size, psize align);
        void* realloc(void* ptr, psize newSize, psize oldSize, psize align);
        void free(void* ptr);

	    IAllocator getParent() const { return m_direct_parent; }
		IAllocator asInterface();
        static TagAllocator* getActiveAllocator();


	    static u32 typeId()
	    {
		    static u32 id = reinterpret_cast<uintptr_t>(&id);
		    return id;
	    }


    private:
	    IAllocator m_direct_parent;
        const std::string_view m_tag;

		static const AllocatorMeta meta;
    };

}