/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 * Test target: FormatTest.cpp
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/memory/Memory.hpp>

#include <tuple>
#include <cstring>

namespace toolkit
{

	[[__deprecated__("use safe version in Formatter class")]] i32 formatRaw(char *buffer, const char *format, ...);

#define FORMAT_BUFFER_SIZE 500

	class FormatBuffer
	{
		friend class Formatter;

	public:
		FormatBuffer(IAllocator &alloc = TkMemory::getDefault());
		~FormatBuffer();

		char *getPointer()
		{
			return m_heap ? m_heap : (char *)&m_inlined[0];
		}
		char *getCurrentPointer()
		{
			return getPointer() + sizeof(char) * m_used;
		}

		psize getUsed() const
		{
			return m_used;
		}

		void append(psize len)
		{
			ensureCapacity(m_used + len);
			m_used += len;
		}

		void appendData(void *data, psize len)
		{
			ensureCapacity(m_used + len);
			char *ptr = getPointer();
			std::memcpy(ptr + m_used, data, len);
			m_used += len;
		}

		void appendCharData(const char *data, psize len)
		{
			ensureCapacity(m_used + len + 1);
			char *ptr = getPointer();
			std::memcpy(ptr + m_used, data, len);
			*(ptr + m_used + len) = '\0';
			m_used += len + 1;
		}

		void clear()
		{
			m_used = 0;
		}

		IAllocator &getAllocator() const
		{
			return m_alloc;
		}

	private:
		char m_inlined[FORMAT_BUFFER_SIZE];
		psize m_capacity;
		psize m_used;
		char *m_heap;
		IAllocator &m_alloc;

		void ensureCapacity(psize needed);
	};

	template <typename T>
	psize formatCustom(FormatBuffer &, const T &)
	{
		return 0;
	}

	class Formatter
	{
	public:
		template <typename T>
		static constexpr bool needs_custom_format =
			!(std::is_arithmetic_v<std::decay_t<T>> ||
			  std::is_same_v<std::decay_t<T>, const char *> ||
			  std::is_same_v<std::decay_t<T>, char *>);

	public:
		static psize formatRaw(FormatBuffer &buf, const char *fmt, ...);

		template <typename... Args>
		static void format(FormatBuffer &buf, const char *formatStr, Args &&...args)
		{
			FormatBuffer dataBuf{buf.getAllocator()};

			auto processedArgs = std::make_tuple(
				[&]
				{
					using T = decltype(args);
					if constexpr (needs_custom_format<T>)
					{
						psize size = formatCustom(dataBuf, args);
						const char *r;
						if (size == 0)
							r = "[unsupported]";
						else
						{
							psize len = size + 1;
							r = static_cast<const char *>(dataBuf.getCurrentPointer() - (size + 1));
						}
						return static_cast<const char *>(r);
					}
					else
					{
						return std::forward<T>(args);
					}
				}()...);

			auto callFormatRaw = [&](auto &&...unpacked) -> i32
			{
				return formatRaw(buf, formatStr, unpacked...);
			};

			std::apply(callFormatRaw, processedArgs);
		}
	};
}

#define FORMAT_CUSTOM(TYPE, FMT, ...)                                  \
	namespace toolkit                                                  \
	{                                                                  \
		template <>                                                    \
		inline psize formatCustom<>(FormatBuffer & buf, const TYPE &t) \
		{                                                              \
			return Formatter::formatRaw(buf, FMT, __VA_ARGS__);        \
		}                                                              \
	}