/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#include "Format.hpp"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#include "Assert.hpp"

namespace toolkit {

	i32 formatRaw(char *buffer, const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);

		i32 r = stbsp_vsprintf(buffer, fmt, args);
		va_end(args);

		return r;
	}

	FormatBuffer::FormatBuffer(IAllocator& alloc)
	    : m_capacity(FORMAT_BUFFER_SIZE), m_used(0), m_heap(nullptr), m_alloc(alloc)
	{}

	FormatBuffer::~FormatBuffer() {
		if (m_heap) {
			TkMemory::free(m_alloc, m_heap);
		}
	}

	void FormatBuffer::ensureCapacity(psize needed) {
		if (needed <= m_capacity)
			return;

		psize new_capacity = std::max(needed, m_capacity * 2);
		char* new_buf = static_cast<char*>(TkMemory::allocate(m_alloc, new_capacity, alignof(char)));
		if (!new_buf)
			TK_ASSERT(false && "alloc failed");

		if (m_used > 0) {
			std::memcpy(new_buf, getPointer(), m_used);
		}

		if (m_heap) {
			TkMemory::free(m_alloc, m_heap);
		}

		m_heap = new_buf;
		m_capacity = new_capacity;
	}


	psize Formatter::formatRaw(FormatBuffer& buf, const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
		psize begin_used = buf.m_used;

		while (true) {
			char* ptr = buf.getPointer();
			psize capacity = buf.getUsed() < buf.m_capacity ? buf.m_capacity : buf.m_capacity;

			va_list args_copy;
			va_copy(args_copy, args);
			int needed = stbsp_vsnprintf(ptr + buf.getUsed(), capacity - buf.getUsed(), fmt, args_copy);
				va_end(args_copy);

			if (needed < 0) {
				va_end(args);
				TK_ASSERT(false && "format error");
			}

			if ((psize)needed < capacity - buf.getUsed()) {
				buf.m_used += needed;
				break;
			}

			buf.ensureCapacity(buf.getUsed() + needed + 1);
		}

        va_end(args);
		//FIX: minus 1 for compatible with appendCharData's '\0' length
		return buf.getUsed() - begin_used - 1;
    }

}