/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/base/Assert.hpp>

#include <type_traits>

namespace toolkit
{

    // This is a template where the Type will be I64 if the T is any unsigned integer and U64 otherwise
    template <typename T, bool = std::is_unsigned<T>::value>
    struct EnumSafeIntegerType
    {
        using Type = i64;
    };

    template <typename T>
    struct EnumSafeIntegerType<T, true>
    {
        using Type = u64;
    };

#define _TK_ENUM_OPERATION_BODY(enumType, regularOperator, a, b)  \
    using EnumInt = std::underlying_type<enumType>::type;         \
    using SafeInt = EnumSafeIntegerType<EnumInt>::Type;           \
    const SafeInt c = SafeInt(a) regularOperator SafeInt(b);      \
    TK_ASSERT(c <= SafeInt(std::numeric_limits<EnumInt>::max())); \
    return enumType(c)

#define _TK_ENUM_OPERATOR(enumType, qualifier, regularOperator, assignmentOperator)                                                        \
    constexpr qualifier enumType operator regularOperator(const enumType a, const enumType b)                                              \
    {                                                                                                                                      \
        _TK_ENUM_OPERATION_BODY(enumType, regularOperator, a, b);                                                                          \
    }                                                                                                                                      \
    constexpr qualifier enumType operator regularOperator(const enumType a, const std::underlying_type<enumType>::type b)                  \
    {                                                                                                                                      \
        _TK_ENUM_OPERATION_BODY(enumType, regularOperator, a, b);                                                                          \
    }                                                                                                                                      \
    constexpr qualifier enumType operator regularOperator(const std::underlying_type<enumType>::type a, const enumType b)                  \
    {                                                                                                                                      \
        _TK_ENUM_OPERATION_BODY(enumType, regularOperator, a, b);                                                                          \
    }                                                                                                                                      \
    qualifier enumType &operator assignmentOperator(enumType &a, const enumType b)                                                         \
    {                                                                                                                                      \
        a = a regularOperator b;                                                                                                           \
        return a;                                                                                                                          \
    }                                                                                                                                      \
    qualifier enumType &operator assignmentOperator(enumType &a, const std::underlying_type<enumType>::type b)                             \
    {                                                                                                                                      \
        a = a regularOperator b;                                                                                                           \
        return a;                                                                                                                          \
    }                                                                                                                                      \
    qualifier std::underlying_type<enumType>::type &operator assignmentOperator(std::underlying_type<enumType>::type &a, const enumType b) \
    {                                                                                                                                      \
        using EnumInt = std::underlying_type<enumType>::type;                                                                              \
        a = EnumInt(a regularOperator b);                                                                                                  \
        return a;                                                                                                                          \
    }

#define _TK_ENUM_UNARAY_OPERATOR(enumType, qualifier, regularOperator)      \
    constexpr qualifier enumType operator regularOperator(const enumType a) \
    {                                                                       \
        using EnumInt = std::underlying_type<enumType>::type;               \
        return enumType(regularOperator EnumInt(a));                        \
    }

#define _TK_ENUM_INCREMENT_DECREMENT(enumType, qualifier) \
    qualifier enumType &operator++(enumType &a)           \
    {                                                     \
        a = a + 1;                                        \
        return a;                                         \
    }                                                     \
    qualifier enumType &operator--(enumType &a)           \
    {                                                     \
        a = a - 1;                                        \
        return a;                                         \
    }                                                     \
    qualifier enumType operator++(enumType &a, int)       \
    {                                                     \
        const enumType old = a;                           \
        ++a;                                              \
        return old;                                       \
    }                                                     \
    qualifier enumType operator--(enumType &a, int)       \
    {                                                     \
        const enumType old = a;                           \
        --a;                                              \
        return old;                                       \
    }

#define _TK_ENUM_NEGATIVE_OPERATOR(enumType, qualifier)       \
    constexpr qualifier bool operator!(const enumType a)      \
    {                                                         \
        using EnumInt = std::underlying_type<enumType>::type; \
        return EnumInt(a) == 0;                               \
    }

#define TK_DECLARE_ENUM_OPERATORS(T)                                                                          \
    inline constexpr bool operator!(T a) { return !(__underlying_type(T))a; }                                 \
    inline constexpr T operator~(T a) { return (T) ~(__underlying_type(T))a; }                                \
    inline constexpr T operator|(T a, T b) { return (T)((__underlying_type(T))a | (__underlying_type(T))b); } \
    inline constexpr T operator&(T a, T b) { return (T)((__underlying_type(T))a & (__underlying_type(T))b); } \
    inline constexpr T operator^(T a, T b) { return (T)((__underlying_type(T))a ^ (__underlying_type(T))b); } \
    inline T &operator|=(T &a, T b) { return a = (T)((__underlying_type(T))a | (__underlying_type(T))b); }    \
    inline T &operator&=(T &a, T b) { return a = (T)((__underlying_type(T))a & (__underlying_type(T))b); }    \
    inline T &operator^=(T &a, T b) { return a = (T)((__underlying_type(T))a ^ (__underlying_type(T))b); }

#define _TK_ENUM_ALLOW_NUMERIC_OPERATIONS(enumType, qualifier) \
    _TK_ENUM_OPERATOR(enumType, qualifier, |, |=)              \
    _TK_ENUM_OPERATOR(enumType, qualifier, &, &=)              \
    _TK_ENUM_OPERATOR(enumType, qualifier, ^, ^=)              \
    _TK_ENUM_OPERATOR(enumType, qualifier, +, +=)              \
    _TK_ENUM_OPERATOR(enumType, qualifier, -, -=)              \
    _TK_ENUM_OPERATOR(enumType, qualifier, *, *=)              \
    _TK_ENUM_OPERATOR(enumType, qualifier, /, /=)              \
    _TK_ENUM_OPERATOR(enumType, qualifier, <<, <<=)            \
    _TK_ENUM_OPERATOR(enumType, qualifier, >>, >>=)            \
    _TK_ENUM_UNARAY_OPERATOR(enumType, qualifier, ~)           \
    _TK_ENUM_INCREMENT_DECREMENT(enumType, qualifier)          \
    _TK_ENUM_NEGATIVE_OPERATOR(enumType, qualifier)

// Implement all those functions that will make a stronly typed enum behave like the old type of enums.
#define TK_ENUM_ALLOW_NUMERIC_OPERATIONS(enumType) _TK_ENUM_ALLOW_NUMERIC_OPERATIONS(enumType, inline)

// Same as TK_ENUM_ALLOW_NUMERIC_OPERATIONS but for enums that are defined in a class.
#define TK_ENUM_ALLOW_NUMERIC_OPERATIONS_FRIEND(enumType) _TK_ENUM_ALLOW_NUMERIC_OPERATIONS(enumType, friend)

    template <typename TEnum>
    class EnumIterableIterator
    {
    public:
        using Type = typename std::underlying_type<TEnum>::type;

        constexpr EnumIterableIterator(TEnum val)
            : m_val(static_cast<Type>(val))
        {
        }

        TEnum operator*() const
        {
            return static_cast<TEnum>(m_val);
        }

        void operator++()
        {
            ++m_val;
        }

        bool operator!=(EnumIterableIterator b) const
        {
            return m_val != b.m_val;
        }

    private:
        Type m_val;
    };

    // Allow an enum to be used in a for range loop.
    template <typename TEnum>
    class EnumIterable
    {
    public:
        using Iterator = EnumIterableIterator<TEnum>;

        constexpr EnumIterable()
            : m_begin(TEnum::kFirst), m_end(TEnum::kCount)
        {
            TK_ASSERT(m_begin <= m_end);
        }

        constexpr EnumIterable(TEnum begin, TEnum end)
            : m_begin(begin), m_end(end)
        {
            TK_ASSERT(m_begin <= m_end);
        }

        Iterator begin() const
        {
            return Iterator(m_begin);
        }

        Iterator end() const
        {
            return Iterator(m_end);
        }

    public:
        TEnum m_begin;
        TEnum m_end;
    };

    template <typename TEnum, typename TBitEnum>
    class EnumBitsIterableIterator
    {
    public:
        using Type = typename std::underlying_type<TBitEnum>::type;

        constexpr EnumBitsIterableIterator(TBitEnum val)
            : m_val(Type(val))
        {
        }

        TEnum operator*() const
        {
            TK_ASSERT(m_val);
            const TEnum out = TEnum(__builtin_ctzll(m_val));
            TK_ASSERT(out >= TEnum::kFirst && out < TEnum::kCount);
            return out;
        }

        void operator++()
        {
            TK_ASSERT(m_val);
            m_val ^= Type(1_u64 << __builtin_ctzll(m_val));
        }

        bool operator!=(EnumBitsIterableIterator b) const
        {
            return m_val != b.m_val;
        }

    private:
        Type m_val;
    };

    // Allow a mask to be used in a for range loop of a compatible enum.
    template <typename TEnum, typename TBitEnum>
    class EnumBitsIterable
    {
    public:
        using Iterator = EnumBitsIterableIterator<TEnum, TBitEnum>;

        constexpr EnumBitsIterable(TBitEnum bits)
            : m_bits(bits)
        {
        }

        Iterator begin() const
        {
            return Iterator(m_bits);
        }

        Iterator end() const
        {
            return Iterator(TBitEnum(0));
        }

    public:
        TBitEnum m_bits;
    };

    // Macros for enums declaration with in-build toString conversion
#define TK_ENUM_TO_STR(x) #x
#define TK_ENUM_TO_STR_FALLBACK ""

#define TK_DECLARE_ENUM_1(name, t0)           \
    enum class name                           \
    {                                         \
        k##t0 = 0                             \
    };                                        \
    static const int name##_const_size = 1;   \
    static const char **name##_const_string() \
    {                                         \
        static const char *str[] = {          \
            TK_ENUM_TO_STR(t0)};              \
        return str;                           \
    }                                         \
    static inline const char *toString(name e) { return name##_const_string()[(int)e]; }

#define TK_DECLARE_ENUM_2(name, t0, t1)              \
    enum class name                                  \
    {                                                \
        k##t0 = 0,                                   \
        k##t1                                        \
    };                                               \
    static const int name##_const_size = 2;          \
    static const char **name##_const_string()        \
    {                                                \
        static const char *str[] = {                 \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1)}; \
        return str;                                  \
    }                                                \
    static inline const char *toString(name e) { return name##_const_string()[(int)e]; }

#define TK_DECLARE_ENUM_3(name, t0, t1, t2)                              \
    enum class name                                                      \
    {                                                                    \
        k##t0 = 0,                                                       \
        k##t1,                                                           \
        k##t2                                                            \
    };                                                                   \
    static const int name##_const_size = 3;                              \
    static const char **name##_const_string()                            \
    {                                                                    \
        static const char *str[] = {                                     \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2)}; \
        return str;                                                      \
    }                                                                    \
    static inline const char *toString(name e) { return name##_const_string()[(int)e]; }

#define TK_DECLARE_ENUM_4(name, t0, t1, t2, t3)                                              \
    enum class name                                                                          \
    {                                                                                        \
        k##t0 = 0,                                                                           \
        k##t1,                                                                               \
        k##t2,                                                                               \
        k##t3                                                                                \
    };                                                                                       \
    static const int name##_const_size = 4;                                                  \
    static const char **name##_const_string()                                                \
    {                                                                                        \
        static const char *str[] = {                                                         \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3)}; \
        return str;                                                                          \
    }                                                                                        \
    static inline const char *toString(name e) { return name##_const_string()[(int)e]; }

#define TK_DECLARE_ENUM_5(name, t0, t1, t2, t3, t4)                                                              \
    enum class name                                                                                              \
    {                                                                                                            \
        k##t0 = 0,                                                                                               \
        k##t1,                                                                                                   \
        k##t2,                                                                                                   \
        k##t3,                                                                                                   \
        k##t4                                                                                                    \
    };                                                                                                           \
    static const int name##_const_size = 5;                                                                      \
    static const char **name##_const_string()                                                                    \
    {                                                                                                            \
        static const char *str[] = {                                                                             \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4)}; \
        return str;                                                                                              \
    }                                                                                                            \
    static inline const char *toString(name e) { return name##_const_string()[(int)e]; }

#define TK_DECLARE_ENUM_6(name, t0, t1, t2, t3, t4, t5)                                                                              \
    enum class name                                                                                                                  \
    {                                                                                                                                \
        k##t0 = 0,                                                                                                                   \
        k##t1,                                                                                                                       \
        k##t2,                                                                                                                       \
        k##t3,                                                                                                                       \
        k##t4,                                                                                                                       \
        k##t5                                                                                                                        \
    };                                                                                                                               \
    static const int name##_const_size = 6;                                                                                          \
    static const char **name##_const_string()                                                                                        \
    {                                                                                                                                \
        static const char *str[] = {                                                                                                 \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4), TK_ENUM_TO_STR(t5)}; \
        return str;                                                                                                                  \
    }                                                                                                                                \
    static inline const char *toString(name e) { return name##_const_string()[(int)e]; }

#define TK_DECLARE_ENUM_7(name, t0, t1, t2, t3, t4, t5, t6)                                                                                              \
    enum class name                                                                                                                                      \
    {                                                                                                                                                    \
        k##t0 = 0,                                                                                                                                       \
        k##t1,                                                                                                                                           \
        k##t2,                                                                                                                                           \
        k##t3,                                                                                                                                           \
        k##t4,                                                                                                                                           \
        k##t5,                                                                                                                                           \
        k##t6                                                                                                                                            \
    };                                                                                                                                                   \
    static const int name##_const_size = 7;                                                                                                              \
    static const char **name##_const_string()                                                                                                            \
    {                                                                                                                                                    \
        static const char *str[] = {                                                                                                                     \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4), TK_ENUM_TO_STR(t5), TK_ENUM_TO_STR(t6)}; \
        return str;                                                                                                                                      \
    }                                                                                                                                                    \
    static inline const char *toString(name e) { return name##_const_string()[(int)e]; }

#define TK_DECLARE_ENUM_8(name, t0, t1, t2, t3, t4, t5, t6, t7)                                                                                                              \
    enum class name                                                                                                                                                          \
    {                                                                                                                                                                        \
        k##t0 = 0,                                                                                                                                                           \
        k##t1,                                                                                                                                                               \
        k##t2,                                                                                                                                                               \
        k##t3,                                                                                                                                                               \
        k##t4,                                                                                                                                                               \
        k##t5,                                                                                                                                                               \
        k##t6,                                                                                                                                                               \
        k##t7                                                                                                                                                                \
    };                                                                                                                                                                       \
    static const int name##_const_size = 8;                                                                                                                                  \
    static const char **name##_const_string()                                                                                                                                \
    {                                                                                                                                                                        \
        static const char *str[] = {                                                                                                                                         \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4), TK_ENUM_TO_STR(t5), TK_ENUM_TO_STR(t6), TK_ENUM_TO_STR(t7)}; \
        return str;                                                                                                                                                          \
    }                                                                                                                                                                        \
    static inline const char *toString(name e) { return name##_const_string()[(int)e]; }

#define TK_DECLARE_ENUM_9(name, t0, t1, t2, t3, t4, t5, t6, t7, t8)                                                                                                                              \
    enum class name                                                                                                                                                                              \
    {                                                                                                                                                                                            \
        k##t0 = 0,                                                                                                                                                                               \
        k##t1,                                                                                                                                                                                   \
        k##t2,                                                                                                                                                                                   \
        k##t3,                                                                                                                                                                                   \
        k##t4,                                                                                                                                                                                   \
        k##t5,                                                                                                                                                                                   \
        k##t6,                                                                                                                                                                                   \
        k##t7,                                                                                                                                                                                   \
        k##t8                                                                                                                                                                                    \
    };                                                                                                                                                                                           \
    static const int name##_const_size = 9;                                                                                                                                                      \
    static const char **name##_const_string()                                                                                                                                                    \
    {                                                                                                                                                                                            \
        static const char *str[] = {                                                                                                                                                             \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4), TK_ENUM_TO_STR(t5), TK_ENUM_TO_STR(t6), TK_ENUM_TO_STR(t7), TK_ENUM_TO_STR(t8)}; \
        return str;                                                                                                                                                                              \
    }                                                                                                                                                                                            \
    static inline const char *toString(name e) { return name##_const_string()[(int)e]; }

#define TK_DECLARE_ENUM_10(name, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9)                                                                                                                                             \
    enum class name                                                                                                                                                                                                  \
    {                                                                                                                                                                                                                \
        k##t0 = 0,                                                                                                                                                                                                   \
        k##t1,                                                                                                                                                                                                       \
        k##t2,                                                                                                                                                                                                       \
        k##t3,                                                                                                                                                                                                       \
        k##t4,                                                                                                                                                                                                       \
        k##t5,                                                                                                                                                                                                       \
        k##t6,                                                                                                                                                                                                       \
        k##t7,                                                                                                                                                                                                       \
        k##t8,                                                                                                                                                                                                       \
        k##t9                                                                                                                                                                                                        \
    };                                                                                                                                                                                                               \
    static const int name##_const_size = 10;                                                                                                                                                                         \
    static const char **name##_const_string()                                                                                                                                                                        \
    {                                                                                                                                                                                                                \
        static const char *str[] = {                                                                                                                                                                                 \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4), TK_ENUM_TO_STR(t5), TK_ENUM_TO_STR(t6), TK_ENUM_TO_STR(t7), TK_ENUM_TO_STR(t8), TK_ENUM_TO_STR(t9)}; \
        return str;                                                                                                                                                                                                  \
    }                                                                                                                                                                                                                \
    static inline const char *toString(name e) { return name##_const_string()[(int)e]; }

#define TK_DECLARE_ENUM_11(name, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10)                                                                                                                                                             \
    enum class name                                                                                                                                                                                                                       \
    {                                                                                                                                                                                                                                     \
        k##t0 = 0,                                                                                                                                                                                                                        \
        k##t1,                                                                                                                                                                                                                            \
        k##t2,                                                                                                                                                                                                                            \
        k##t3,                                                                                                                                                                                                                            \
        k##t4,                                                                                                                                                                                                                            \
        k##t5,                                                                                                                                                                                                                            \
        k##t6,                                                                                                                                                                                                                            \
        k##t7,                                                                                                                                                                                                                            \
        k##t8,                                                                                                                                                                                                                            \
        k##t9,                                                                                                                                                                                                                            \
        k##t10                                                                                                                                                                                                                            \
    };                                                                                                                                                                                                                                    \
    static const int name##_const_size = 11;                                                                                                                                                                                              \
    static const char **name##_const_string()                                                                                                                                                                                             \
    {                                                                                                                                                                                                                                     \
        static const char *str[] = {                                                                                                                                                                                                      \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4), TK_ENUM_TO_STR(t5), TK_ENUM_TO_STR(t6), TK_ENUM_TO_STR(t7), TK_ENUM_TO_STR(t8), TK_ENUM_TO_STR(t9), TK_ENUM_TO_STR(t10)}; \
        return str;                                                                                                                                                                                                                       \
    }                                                                                                                                                                                                                                     \
    static inline const char *toString(name e) { return name##_const_string()[(int)e]; }

#define TK_DECLARE_ENUM_12(name, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11)                                                                                                                                                                             \
    enum class name                                                                                                                                                                                                                                            \
    {                                                                                                                                                                                                                                                          \
        k##t0 = 0,                                                                                                                                                                                                                                             \
        k##t1,                                                                                                                                                                                                                                                 \
        k##t2,                                                                                                                                                                                                                                                 \
        k##t3,                                                                                                                                                                                                                                                 \
        k##t4,                                                                                                                                                                                                                                                 \
        k##t5,                                                                                                                                                                                                                                                 \
        k##t6,                                                                                                                                                                                                                                                 \
        k##t7,                                                                                                                                                                                                                                                 \
        k##t8,                                                                                                                                                                                                                                                 \
        k##t9,                                                                                                                                                                                                                                                 \
        k##t10,                                                                                                                                                                                                                                                \
        k##t11                                                                                                                                                                                                                                                 \
    };                                                                                                                                                                                                                                                         \
    static const int name##_const_size = 12;                                                                                                                                                                                                                   \
    static const char **name##_const_string()                                                                                                                                                                                                                  \
    {                                                                                                                                                                                                                                                          \
        static const char *str[] = {                                                                                                                                                                                                                           \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4), TK_ENUM_TO_STR(t5), TK_ENUM_TO_STR(t6), TK_ENUM_TO_STR(t7), TK_ENUM_TO_STR(t8), TK_ENUM_TO_STR(t9), TK_ENUM_TO_STR(t10), TK_ENUM_TO_STR(t11)}; \
        return str;                                                                                                                                                                                                                                            \
    }                                                                                                                                                                                                                                                          \
    static inline const char *toString(name e) { return name##_const_string()[(int)e]; }

#define TK_DECLARE_ENUM_13(name, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12)                                                                                                                                                                                             \
    enum class name                                                                                                                                                                                                                                                                 \
    {                                                                                                                                                                                                                                                                               \
        k##t0 = 0,                                                                                                                                                                                                                                                                  \
        k##t1,                                                                                                                                                                                                                                                                      \
        k##t2,                                                                                                                                                                                                                                                                      \
        k##t3,                                                                                                                                                                                                                                                                      \
        k##t4,                                                                                                                                                                                                                                                                      \
        k##t5,                                                                                                                                                                                                                                                                      \
        k##t6,                                                                                                                                                                                                                                                                      \
        k##t7,                                                                                                                                                                                                                                                                      \
        k##t8,                                                                                                                                                                                                                                                                      \
        k##t9,                                                                                                                                                                                                                                                                      \
        k##t10,                                                                                                                                                                                                                                                                     \
        k##t11,                                                                                                                                                                                                                                                                     \
        k##t12                                                                                                                                                                                                                                                                      \
    };                                                                                                                                                                                                                                                                              \
    static const int name##_const_size = 13;                                                                                                                                                                                                                                        \
    static const char **name##_const_string()                                                                                                                                                                                                                                       \
    {                                                                                                                                                                                                                                                                               \
        static const char *str[] = {                                                                                                                                                                                                                                                \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4), TK_ENUM_TO_STR(t5), TK_ENUM_TO_STR(t6), TK_ENUM_TO_STR(t7), TK_ENUM_TO_STR(t8), TK_ENUM_TO_STR(t9), TK_ENUM_TO_STR(t10), TK_ENUM_TO_STR(t11), TK_ENUM_TO_STR(t12)}; \
        return str;                                                                                                                                                                                                                                                                 \
    }                                                                                                                                                                                                                                                                               \
    static inline const char *toString(name e) { return name##_const_string()[(int)e]; }

#define TK_DECLARE_ENUM_14(name, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13)                                                                                                                                                                                                             \
    enum class name                                                                                                                                                                                                                                                                                      \
    {                                                                                                                                                                                                                                                                                                    \
        k##t0 = 0,                                                                                                                                                                                                                                                                                       \
        k##t1,                                                                                                                                                                                                                                                                                           \
        k##t2,                                                                                                                                                                                                                                                                                           \
        k##t3,                                                                                                                                                                                                                                                                                           \
        k##t4,                                                                                                                                                                                                                                                                                           \
        k##t5,                                                                                                                                                                                                                                                                                           \
        k##t6,                                                                                                                                                                                                                                                                                           \
        k##t7,                                                                                                                                                                                                                                                                                           \
        k##t8,                                                                                                                                                                                                                                                                                           \
        k##t9,                                                                                                                                                                                                                                                                                           \
        k##t10,                                                                                                                                                                                                                                                                                          \
        k##t11,                                                                                                                                                                                                                                                                                          \
        k##t12,                                                                                                                                                                                                                                                                                          \
        k##t13                                                                                                                                                                                                                                                                                           \
    };                                                                                                                                                                                                                                                                                                   \
    static const int name##_const_size = 14;                                                                                                                                                                                                                                                             \
    static const char **name##_const_string()                                                                                                                                                                                                                                                            \
    {                                                                                                                                                                                                                                                                                                    \
        static const char *str[] = {                                                                                                                                                                                                                                                                     \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4), TK_ENUM_TO_STR(t5), TK_ENUM_TO_STR(t6), TK_ENUM_TO_STR(t7), TK_ENUM_TO_STR(t8), TK_ENUM_TO_STR(t9), TK_ENUM_TO_STR(t10), TK_ENUM_TO_STR(t11), TK_ENUM_TO_STR(t12), TK_ENUM_TO_STR(t13)}; \
        return str;                                                                                                                                                                                                                                                                                      \
    }                                                                                                                                                                                                                                                                                                    \
    static inline const char *toString(name e) { return name##_const_string()[(int)e]; }

#define TK_DECLARE_ENUM_15(name, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14)                                                                                                                                                                                                                             \
    enum class name                                                                                                                                                                                                                                                                                                           \
    {                                                                                                                                                                                                                                                                                                                         \
        k##t0 = 0,                                                                                                                                                                                                                                                                                                            \
        k##t1,                                                                                                                                                                                                                                                                                                                \
        k##t2,                                                                                                                                                                                                                                                                                                                \
        k##t3,                                                                                                                                                                                                                                                                                                                \
        k##t4,                                                                                                                                                                                                                                                                                                                \
        k##t5,                                                                                                                                                                                                                                                                                                                \
        k##t6,                                                                                                                                                                                                                                                                                                                \
        k##t7,                                                                                                                                                                                                                                                                                                                \
        k##t8,                                                                                                                                                                                                                                                                                                                \
        k##t9,                                                                                                                                                                                                                                                                                                                \
        k##t10,                                                                                                                                                                                                                                                                                                               \
        k##t11,                                                                                                                                                                                                                                                                                                               \
        k##t12,                                                                                                                                                                                                                                                                                                               \
        k##t13,                                                                                                                                                                                                                                                                                                               \
        k##t14                                                                                                                                                                                                                                                                                                                \
    };                                                                                                                                                                                                                                                                                                                        \
    static const int name##_const_size = 15;                                                                                                                                                                                                                                                                                  \
    static const char **name##_const_string()                                                                                                                                                                                                                                                                                 \
    {                                                                                                                                                                                                                                                                                                                         \
        static const char *str[] = {                                                                                                                                                                                                                                                                                          \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4), TK_ENUM_TO_STR(t5), TK_ENUM_TO_STR(t6), TK_ENUM_TO_STR(t7), TK_ENUM_TO_STR(t8), TK_ENUM_TO_STR(t9), TK_ENUM_TO_STR(t10), TK_ENUM_TO_STR(t11), TK_ENUM_TO_STR(t12), TK_ENUM_TO_STR(t13), TK_ENUM_TO_STR(t14)}; \
        return str;                                                                                                                                                                                                                                                                                                           \
    }                                                                                                                                                                                                                                                                                                                         \
    static inline const char *toString(name e) { return name##_const_string()[(int)e]; }

#define TK_DECLARE_ENUM_16(name, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15)                                                                                                                                                                                                                                             \
    enum class name                                                                                                                                                                                                                                                                                                                                \
    {                                                                                                                                                                                                                                                                                                                                              \
        k##t0 = 0,                                                                                                                                                                                                                                                                                                                                 \
        k##t1,                                                                                                                                                                                                                                                                                                                                     \
        k##t2,                                                                                                                                                                                                                                                                                                                                     \
        k##t3,                                                                                                                                                                                                                                                                                                                                     \
        k##t4,                                                                                                                                                                                                                                                                                                                                     \
        k##t5,                                                                                                                                                                                                                                                                                                                                     \
        k##t6,                                                                                                                                                                                                                                                                                                                                     \
        k##t7,                                                                                                                                                                                                                                                                                                                                     \
        k##t8,                                                                                                                                                                                                                                                                                                                                     \
        k##t9,                                                                                                                                                                                                                                                                                                                                     \
        k##t10,                                                                                                                                                                                                                                                                                                                                    \
        k##t11,                                                                                                                                                                                                                                                                                                                                    \
        k##t12,                                                                                                                                                                                                                                                                                                                                    \
        k##t13,                                                                                                                                                                                                                                                                                                                                    \
        k##t14,                                                                                                                                                                                                                                                                                                                                    \
        k##t15                                                                                                                                                                                                                                                                                                                                     \
    };                                                                                                                                                                                                                                                                                                                                             \
    static const int name##_const_size = 16;                                                                                                                                                                                                                                                                                                       \
    static const char **name##_const_string()                                                                                                                                                                                                                                                                                                      \
    {                                                                                                                                                                                                                                                                                                                                              \
        static const char *str[] = {                                                                                                                                                                                                                                                                                                               \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4), TK_ENUM_TO_STR(t5), TK_ENUM_TO_STR(t6), TK_ENUM_TO_STR(t7), TK_ENUM_TO_STR(t8), TK_ENUM_TO_STR(t9), TK_ENUM_TO_STR(t10), TK_ENUM_TO_STR(t11), TK_ENUM_TO_STR(t12), TK_ENUM_TO_STR(t13), TK_ENUM_TO_STR(t14), TK_ENUM_TO_STR(t15)}; \
        return str;                                                                                                                                                                                                                                                                                                                                \
    }                                                                                                                                                                                                                                                                                                                                              \
    static inline const char *toString(name e) { return name##_const_string()[(int)e]; }

#define TK_DECLARE_ENUM_17(name, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16)                                                                                                                                                                                                                                                             \
    enum class name                                                                                                                                                                                                                                                                                                                                                     \
    {                                                                                                                                                                                                                                                                                                                                                                   \
        k##t0 = 0,                                                                                                                                                                                                                                                                                                                                                      \
        k##t1,                                                                                                                                                                                                                                                                                                                                                          \
        k##t2,                                                                                                                                                                                                                                                                                                                                                          \
        k##t3,                                                                                                                                                                                                                                                                                                                                                          \
        k##t4,                                                                                                                                                                                                                                                                                                                                                          \
        k##t5,                                                                                                                                                                                                                                                                                                                                                          \
        k##t6,                                                                                                                                                                                                                                                                                                                                                          \
        k##t7,                                                                                                                                                                                                                                                                                                                                                          \
        k##t8,                                                                                                                                                                                                                                                                                                                                                          \
        k##t9,                                                                                                                                                                                                                                                                                                                                                          \
        k##t10,                                                                                                                                                                                                                                                                                                                                                         \
        k##t11,                                                                                                                                                                                                                                                                                                                                                         \
        k##t12,                                                                                                                                                                                                                                                                                                                                                         \
        k##t13,                                                                                                                                                                                                                                                                                                                                                         \
        k##t14,                                                                                                                                                                                                                                                                                                                                                         \
        k##t15,                                                                                                                                                                                                                                                                                                                                                         \
        k##t16                                                                                                                                                                                                                                                                                                                                                          \
    };                                                                                                                                                                                                                                                                                                                                                                  \
    static const int name##_const_size = 17;                                                                                                                                                                                                                                                                                                                            \
    static const char **name##_const_string()                                                                                                                                                                                                                                                                                                                           \
    {                                                                                                                                                                                                                                                                                                                                                                   \
        static const char *str[] = {                                                                                                                                                                                                                                                                                                                                    \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4), TK_ENUM_TO_STR(t5), TK_ENUM_TO_STR(t6), TK_ENUM_TO_STR(t7), TK_ENUM_TO_STR(t8), TK_ENUM_TO_STR(t9), TK_ENUM_TO_STR(t10), TK_ENUM_TO_STR(t11), TK_ENUM_TO_STR(t12), TK_ENUM_TO_STR(t13), TK_ENUM_TO_STR(t14), TK_ENUM_TO_STR(t15), TK_ENUM_TO_STR(t16)}; \
        return str;                                                                                                                                                                                                                                                                                                                                                     \
    }                                                                                                                                                                                                                                                                                                                                                                   \
    static inline const char *toString(name e) { return name##_const_string()[(int)e]; }

#define TK_DECLARE_ENUM_18(name, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15, t16, t17)                                                                                                                                                                                                                                                                             \
    enum class name                                                                                                                                                                                                                                                                                                                                                                          \
    {                                                                                                                                                                                                                                                                                                                                                                                        \
        k##t0 = 0,                                                                                                                                                                                                                                                                                                                                                                           \
        k##t1,                                                                                                                                                                                                                                                                                                                                                                               \
        k##t2,                                                                                                                                                                                                                                                                                                                                                                               \
        k##t3,                                                                                                                                                                                                                                                                                                                                                                               \
        k##t4,                                                                                                                                                                                                                                                                                                                                                                               \
        k##t5,                                                                                                                                                                                                                                                                                                                                                                               \
        k##t6,                                                                                                                                                                                                                                                                                                                                                                               \
        k##t7,                                                                                                                                                                                                                                                                                                                                                                               \
        k##t8,                                                                                                                                                                                                                                                                                                                                                                               \
        k##t9,                                                                                                                                                                                                                                                                                                                                                                               \
        k##t10,                                                                                                                                                                                                                                                                                                                                                                              \
        k##t11,                                                                                                                                                                                                                                                                                                                                                                              \
        k##t12,                                                                                                                                                                                                                                                                                                                                                                              \
        k##t13,                                                                                                                                                                                                                                                                                                                                                                              \
        k##t14,                                                                                                                                                                                                                                                                                                                                                                              \
        k##t15,                                                                                                                                                                                                                                                                                                                                                                              \
        k##t16,                                                                                                                                                                                                                                                                                                                                                                              \
        k##t17                                                                                                                                                                                                                                                                                                                                                                               \
    };                                                                                                                                                                                                                                                                                                                                                                                       \
    static const int name##_const_size = 17;                                                                                                                                                                                                                                                                                                                                                 \
    static const char **name##_const_string()                                                                                                                                                                                                                                                                                                                                                \
    {                                                                                                                                                                                                                                                                                                                                                                                        \
        static const char *str[] = {                                                                                                                                                                                                                                                                                                                                                         \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4), TK_ENUM_TO_STR(t5), TK_ENUM_TO_STR(t6), TK_ENUM_TO_STR(t7), TK_ENUM_TO_STR(t8), TK_ENUM_TO_STR(t9), TK_ENUM_TO_STR(t10), TK_ENUM_TO_STR(t11), TK_ENUM_TO_STR(t12), TK_ENUM_TO_STR(t13), TK_ENUM_TO_STR(t14), TK_ENUM_TO_STR(t15), TK_ENUM_TO_STR(t16), TK_ENUM_TO_STR(t17)}; \
        return str;                                                                                                                                                                                                                                                                                                                                                                          \
    }                                                                                                                                                                                                                                                                                                                                                                                        \
    static inline const char *toString(name e) { return name##_const_string()[(int)e]; }

    // ..and those with base type and base value

#define TK_DECLARE_ENUM_EX_1(name, baseType, baseVal, t0) \
    enum class name : baseType                            \
    {                                                     \
        k##t0 = baseVal                                   \
    };                                                    \
    static const int name##_const_size = 1;               \
    static const char **name##_const_string()             \
    {                                                     \
        static const char *str[] = {                      \
            TK_ENUM_TO_STR(t0)};                          \
        return str;                                       \
    }                                                     \
    static inline const char *toString(name e) { return name##_const_string()[(int)e - baseVal]; }

#define TK_DECLARE_ENUM_EX_2(name, baseType, baseVal, t0, t1) \
    enum class name : baseType                                \
    {                                                         \
        k##t0 = baseVal,                                      \
        k##t1                                                 \
    };                                                        \
    static const int name##_const_size = 2;                   \
    static const char **name##_const_string()                 \
    {                                                         \
        static const char *str[] = {                          \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1)};          \
        return str;                                           \
    }                                                         \
    static inline const char *toString(name e) { return name##_const_string()[(int)e - baseVal]; }

#define TK_DECLARE_ENUM_EX_3(name, baseType, baseVal, t0, t1, t2)        \
    enum class name : baseType                                           \
    {                                                                    \
        k##t0 = baseVal,                                                 \
        k##t1,                                                           \
        k##t2                                                            \
    };                                                                   \
    static const int name##_const_size = 3;                              \
    static const char **name##_const_string()                            \
    {                                                                    \
        static const char *str[] = {                                     \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2)}; \
        return str;                                                      \
    }                                                                    \
    static inline const char *toString(name e) { return name##_const_string()[(int)e - baseVal]; }

#define TK_DECLARE_ENUM_EX_4(name, baseType, baseVal, t0, t1, t2, t3)                        \
    enum class name : baseType                                                               \
    {                                                                                        \
        k##t0 = baseVal,                                                                     \
        k##t1,                                                                               \
        k##t2,                                                                               \
        k##t3                                                                                \
    };                                                                                       \
    static const int name##_const_size = 4;                                                  \
    static const char **name##_const_string()                                                \
    {                                                                                        \
        static const char *str[] = {                                                         \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3)}; \
        return str;                                                                          \
    }                                                                                        \
    static inline const char *toString(name e) { return name##_const_string()[(int)e - baseVal]; }

#define TK_DECLARE_ENUM_EX_5(name, baseType, baseVal, t0, t1, t2, t3, t4)                                        \
    enum class name : baseType                                                                                   \
    {                                                                                                            \
        k##t0 = baseVal,                                                                                         \
        k##t1,                                                                                                   \
        k##t2,                                                                                                   \
        k##t3,                                                                                                   \
        k##t4                                                                                                    \
    };                                                                                                           \
    static const int name##_const_size = 5;                                                                      \
    static const char **name##_const_string()                                                                    \
    {                                                                                                            \
        static const char *str[] = {                                                                             \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4)}; \
        return str;                                                                                              \
    }                                                                                                            \
    static inline const char *toString(name e) { return name##_const_string()[(int)e - baseVal]; }

#define TK_DECLARE_ENUM_EX_6(name, baseType, baseVal, t0, t1, t2, t3, t4, t5)                                                        \
    enum class name : baseType                                                                                                       \
    {                                                                                                                                \
        k##t0 = baseVal,                                                                                                             \
        k##t1,                                                                                                                       \
        k##t2,                                                                                                                       \
        k##t3,                                                                                                                       \
        k##t4,                                                                                                                       \
        k##t5                                                                                                                        \
    };                                                                                                                               \
    static const int name##_const_size = 6;                                                                                          \
    static const char **name##_const_string()                                                                                        \
    {                                                                                                                                \
        static const char *str[] = {                                                                                                 \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4), TK_ENUM_TO_STR(t5)}; \
        return str;                                                                                                                  \
    }                                                                                                                                \
    static inline const char *toString(name e) { return name##_const_string()[(int)e - baseVal]; }

#define TK_DECLARE_ENUM_EX_7(name, baseType, baseVal, t0, t1, t2, t3, t4, t5, t6)                                                                        \
    enum class name : baseType                                                                                                                           \
    {                                                                                                                                                    \
        k##t0 = baseVal,                                                                                                                                 \
        k##t1,                                                                                                                                           \
        k##t2,                                                                                                                                           \
        k##t3,                                                                                                                                           \
        k##t4,                                                                                                                                           \
        k##t5,                                                                                                                                           \
        k##t6                                                                                                                                            \
    };                                                                                                                                                   \
    static const int name##_const_size = 7;                                                                                                              \
    static const char **name##_const_string()                                                                                                            \
    {                                                                                                                                                    \
        static const char *str[] = {                                                                                                                     \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4), TK_ENUM_TO_STR(t5), TK_ENUM_TO_STR(t6)}; \
        return str;                                                                                                                                      \
    }                                                                                                                                                    \
    static inline const char *toString(name e) { return name##_const_string()[(int)e - baseVal]; }

#define TK_DECLARE_ENUM_EX_8(name, baseType, baseVal, t0, t1, t2, t3, t4, t5, t6, t7)                                                                                        \
    enum class name : baseType                                                                                                                                               \
    {                                                                                                                                                                        \
        k##t0 = baseVal,                                                                                                                                                     \
        k##t1,                                                                                                                                                               \
        k##t2,                                                                                                                                                               \
        k##t3,                                                                                                                                                               \
        k##t4,                                                                                                                                                               \
        k##t5,                                                                                                                                                               \
        k##t6,                                                                                                                                                               \
        k##t7                                                                                                                                                                \
    };                                                                                                                                                                       \
    static const int name##_const_size = 8;                                                                                                                                  \
    static const char **name##_const_string()                                                                                                                                \
    {                                                                                                                                                                        \
        static const char *str[] = {                                                                                                                                         \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4), TK_ENUM_TO_STR(t5), TK_ENUM_TO_STR(t6), TK_ENUM_TO_STR(t7)}; \
        return str;                                                                                                                                                          \
    }                                                                                                                                                                        \
    static inline const char *toString(name e) { return name##_const_string()[(int)e - baseVal]; }

#define TK_DECLARE_ENUM_EX_9(name, baseType, baseVal, t0, t1, t2, t3, t4, t5, t6, t7, t8)                                                                                                        \
    enum class name : baseType                                                                                                                                                                   \
    {                                                                                                                                                                                            \
        k##t0 = baseVal,                                                                                                                                                                         \
        k##t1,                                                                                                                                                                                   \
        k##t2,                                                                                                                                                                                   \
        k##t3,                                                                                                                                                                                   \
        k##t4,                                                                                                                                                                                   \
        k##t5,                                                                                                                                                                                   \
        k##t6,                                                                                                                                                                                   \
        k##t7,                                                                                                                                                                                   \
        k##t8                                                                                                                                                                                    \
    };                                                                                                                                                                                           \
    static const int name##_const_size = 9;                                                                                                                                                      \
    static const char **name##_const_string()                                                                                                                                                    \
    {                                                                                                                                                                                            \
        static const char *str[] = {                                                                                                                                                             \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4), TK_ENUM_TO_STR(t5), TK_ENUM_TO_STR(t6), TK_ENUM_TO_STR(t7), TK_ENUM_TO_STR(t8)}; \
        return str;                                                                                                                                                                              \
    }                                                                                                                                                                                            \
    static inline const char *toString(name e) { return name##_const_string()[(int)e - baseVal]; }

#define TK_DECLARE_ENUM_EX_10(name, baseType, baseVal, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9)                                                                                                                       \
    enum class name : baseType                                                                                                                                                                                       \
    {                                                                                                                                                                                                                \
        k##t0 = baseVal,                                                                                                                                                                                             \
        k##t1,                                                                                                                                                                                                       \
        k##t2,                                                                                                                                                                                                       \
        k##t3,                                                                                                                                                                                                       \
        k##t4,                                                                                                                                                                                                       \
        k##t5,                                                                                                                                                                                                       \
        k##t6,                                                                                                                                                                                                       \
        k##t7,                                                                                                                                                                                                       \
        k##t8,                                                                                                                                                                                                       \
        k##t9                                                                                                                                                                                                        \
    };                                                                                                                                                                                                               \
    static const int name##_const_size = 10;                                                                                                                                                                         \
    static const char **name##_const_string()                                                                                                                                                                        \
    {                                                                                                                                                                                                                \
        static const char *str[] = {                                                                                                                                                                                 \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4), TK_ENUM_TO_STR(t5), TK_ENUM_TO_STR(t6), TK_ENUM_TO_STR(t7), TK_ENUM_TO_STR(t8), TK_ENUM_TO_STR(t9)}; \
        return str;                                                                                                                                                                                                  \
    }                                                                                                                                                                                                                \
    static inline const char *toString(name e) { return name##_const_string()[(int)e - baseVal]; }

#define TK_DECLARE_ENUM_EX_11(name, baseType, baseVal, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10)                                                                                                                                       \
    enum class name : baseType                                                                                                                                                                                                            \
    {                                                                                                                                                                                                                                     \
        k##t0 = baseVal,                                                                                                                                                                                                                  \
        k##t1,                                                                                                                                                                                                                            \
        k##t2,                                                                                                                                                                                                                            \
        k##t3,                                                                                                                                                                                                                            \
        k##t4,                                                                                                                                                                                                                            \
        k##t5,                                                                                                                                                                                                                            \
        k##t6,                                                                                                                                                                                                                            \
        k##t7,                                                                                                                                                                                                                            \
        k##t8,                                                                                                                                                                                                                            \
        k##t9,                                                                                                                                                                                                                            \
        k##t10                                                                                                                                                                                                                            \
    };                                                                                                                                                                                                                                    \
    static const int name##_const_size = 11;                                                                                                                                                                                              \
    static const char **name##_const_string()                                                                                                                                                                                             \
    {                                                                                                                                                                                                                                     \
        static const char *str[] = {                                                                                                                                                                                                      \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4), TK_ENUM_TO_STR(t5), TK_ENUM_TO_STR(t6), TK_ENUM_TO_STR(t7), TK_ENUM_TO_STR(t8), TK_ENUM_TO_STR(t9), TK_ENUM_TO_STR(t10)}; \
        return str;                                                                                                                                                                                                                       \
    }                                                                                                                                                                                                                                     \
    static inline const char *toString(name e) { return name##_const_string()[(int)e - baseVal]; }

#define TK_DECLARE_ENUM_EX_12(name, baseType, baseVal, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11)                                                                                                                                                       \
    enum class name : baseType                                                                                                                                                                                                                                 \
    {                                                                                                                                                                                                                                                          \
        k##t0 = baseVal,                                                                                                                                                                                                                                       \
        k##t1,                                                                                                                                                                                                                                                 \
        k##t2,                                                                                                                                                                                                                                                 \
        k##t3,                                                                                                                                                                                                                                                 \
        k##t4,                                                                                                                                                                                                                                                 \
        k##t5,                                                                                                                                                                                                                                                 \
        k##t6,                                                                                                                                                                                                                                                 \
        k##t7,                                                                                                                                                                                                                                                 \
        k##t8,                                                                                                                                                                                                                                                 \
        k##t9,                                                                                                                                                                                                                                                 \
        k##t10,                                                                                                                                                                                                                                                \
        k##t11                                                                                                                                                                                                                                                 \
    };                                                                                                                                                                                                                                                         \
    static const int name##_const_size = 11;                                                                                                                                                                                                                   \
    static const char **name##_const_string()                                                                                                                                                                                                                  \
    {                                                                                                                                                                                                                                                          \
        static const char *str[] = {                                                                                                                                                                                                                           \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4), TK_ENUM_TO_STR(t5), TK_ENUM_TO_STR(t6), TK_ENUM_TO_STR(t7), TK_ENUM_TO_STR(t8), TK_ENUM_TO_STR(t9), TK_ENUM_TO_STR(t10), TK_ENUM_TO_STR(t11)}; \
        return str;                                                                                                                                                                                                                                            \
    }                                                                                                                                                                                                                                                          \
    static inline const char *toString(name e) { return name##_const_string()[(int)e - baseVal]; }

    // ..and those for flags

#define TK_DECLARE_ENUM_FLAGS_1(name, baseType, t0, v0)  \
    enum class name : baseType                           \
    {                                                    \
        k##t0 = v0                                       \
    };                                                   \
    static const int name##_const_size = 1;              \
    static const char **name##_const_string()            \
    {                                                    \
        static const char *str[] = {                     \
            TK_ENUM_TO_STR(t0)};                         \
        return str;                                      \
    }                                                    \
    static inline const char *toString(name e)           \
    {                                                    \
        static baseType val[] = {                        \
            (baseType)(v0)};                             \
        for (baseType i = 0; i < name##_const_size; i++) \
        {                                                \
            if (val[i] == (baseType)e)                   \
                return name##_const_string()[(int)i];    \
        }                                                \
        return TK_ENUM_TO_STR_FALLBACK;                  \
    }

#define TK_DECLARE_ENUM_FLAGS_2(name, baseType, t0, v0, t1, v1) \
    enum class name : baseType                                  \
    {                                                           \
        k##t0 = v0,                                             \
        k##t1 = v1                                              \
    };                                                          \
    static const int name##_const_size = 2;                     \
    static const char **name##_const_string()                   \
    {                                                           \
        static const char *str[] = {                            \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1)};            \
        return str;                                             \
    }                                                           \
    static inline const char *toString(name e)                  \
    {                                                           \
        static baseType val[] = {                               \
            (baseType)(v0), (baseType)(v1)};                    \
        for (baseType i = 0; i < name##_const_size; i++)        \
        {                                                       \
            if (val[i] == (baseType)e)                          \
                return name##_const_string()[(int)i];           \
        }                                                       \
        return TK_ENUM_TO_STR_FALLBACK;                         \
    }

#define TK_DECLARE_ENUM_FLAGS_3(name, baseType, t0, v0, t1, v1, t2, v2)  \
    enum class name : baseType                                           \
    {                                                                    \
        k##t0 = v0,                                                      \
        k##t1 = v1,                                                      \
        k##t2 = v2                                                       \
    };                                                                   \
    static const int name##_const_size = 3;                              \
    static const char **name##_const_string()                            \
    {                                                                    \
        static const char *str[] = {                                     \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2)}; \
        return str;                                                      \
    }                                                                    \
    static inline const char *toString(name e)                           \
    {                                                                    \
        static baseType val[] = {                                        \
            (baseType)(v0), (baseType)(v1), (baseType)(v2)};             \
        for (baseType i = 0; i < name##_const_size; i++)                 \
        {                                                                \
            if (val[i] == (baseType)e)                                   \
                return name##_const_string()[(int)i];                    \
        }                                                                \
        return TK_ENUM_TO_STR_FALLBACK;                                  \
    }

#define TK_DECLARE_ENUM_FLAGS_4(name, baseType, t0, v0, t1, v1, t2, v2, t3, v3)              \
    enum class name : baseType                                                               \
    {                                                                                        \
        k##t0 = v0,                                                                          \
        k##t1 = v1,                                                                          \
        k##t2 = v2,                                                                          \
        k##t3 = v3                                                                           \
    };                                                                                       \
    static const int name##_const_size = 4;                                                  \
    static const char **name##_const_string()                                                \
    {                                                                                        \
        static const char *str[] = {                                                         \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3)}; \
        return str;                                                                          \
    }                                                                                        \
    static inline const char *toString(name e)                                               \
    {                                                                                        \
        static baseType val[] = {                                                            \
            (baseType)(v0), (baseType)(v1), (baseType)(v2), (baseType)(v3)};                 \
        for (baseType i = 0; i < name##_const_size; i++)                                     \
        {                                                                                    \
            if (val[i] == (baseType)e)                                                       \
                return name##_const_string()[(int)i];                                        \
        }                                                                                    \
        return TK_ENUM_TO_STR_FALLBACK;                                                      \
    }

#define TK_DECLARE_ENUM_FLAGS_5(name, baseType, t0, v0, t1, v1, t2, v2, t3, v3, t4, v4)                          \
    enum class name : baseType                                                                                   \
    {                                                                                                            \
        k##t0 = v0,                                                                                              \
        k##t1 = v1,                                                                                              \
        k##t2 = v2,                                                                                              \
        k##t3 = v3,                                                                                              \
        k##t4 = v4                                                                                               \
    };                                                                                                           \
    static const int name##_const_size = 5;                                                                      \
    static const char **name##_const_string()                                                                    \
    {                                                                                                            \
        static const char *str[] = {                                                                             \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4)}; \
        return str;                                                                                              \
    }                                                                                                            \
    static inline const char *toString(name e)                                                                   \
    {                                                                                                            \
        static baseType val[] = {                                                                                \
            (baseType)(v0), (baseType)(v1), (baseType)(v2), (baseType)(v3), (baseType)(v4)};                     \
        for (baseType i = 0; i < name##_const_size; i++)                                                         \
        {                                                                                                        \
            if (val[i] == (baseType)e)                                                                           \
                return name##_const_string()[(int)i];                                                            \
        }                                                                                                        \
        return TK_ENUM_TO_STR_FALLBACK;                                                                          \
    }

#define TK_DECLARE_ENUM_FLAGS_6(name, baseType, t0, v0, t1, v1, t2, v2, t3, v3, t4, v4, t5, v5)                                      \
    enum class name : baseType                                                                                                       \
    {                                                                                                                                \
        k##t0 = v0,                                                                                                                  \
        k##t1 = v1,                                                                                                                  \
        k##t2 = v2,                                                                                                                  \
        k##t3 = v3,                                                                                                                  \
        k##t4 = v4,                                                                                                                  \
        k##t5 = v5                                                                                                                   \
    };                                                                                                                               \
    static const int name##_const_size = 6;                                                                                          \
    static const char **name##_const_string()                                                                                        \
    {                                                                                                                                \
        static const char *str[] = {                                                                                                 \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4), TK_ENUM_TO_STR(t5)}; \
        return str;                                                                                                                  \
    }                                                                                                                                \
    static inline const char *toString(name e)                                                                                       \
    {                                                                                                                                \
        static baseType val[] = {                                                                                                    \
            (baseType)(v0), (baseType)(v1), (baseType)(v2), (baseType)(v3), (baseType)(v4), (baseType)(v5)};                         \
        for (baseType i = 0; i < name##_const_size; i++)                                                                             \
        {                                                                                                                            \
            if (val[i] == (baseType)e)                                                                                               \
                return name##_const_string()[(int)i];                                                                                \
        }                                                                                                                            \
        return TK_ENUM_TO_STR_FALLBACK;                                                                                              \
    }

#define TK_DECLARE_ENUM_FLAGS_7(name, baseType, t0, v0, t1, v1, t2, v2, t3, v3, t4, v4, t5, v5, t6, v6)                                                  \
    enum class name : baseType                                                                                                                           \
    {                                                                                                                                                    \
        k##t0 = v0,                                                                                                                                      \
        k##t1 = v1,                                                                                                                                      \
        k##t2 = v2,                                                                                                                                      \
        k##t3 = v3,                                                                                                                                      \
        k##t4 = v4,                                                                                                                                      \
        k##t5 = v5,                                                                                                                                      \
        k##t6 = v6                                                                                                                                       \
    };                                                                                                                                                   \
    static const int name##_const_size = 7;                                                                                                              \
    static const char **name##_const_string()                                                                                                            \
    {                                                                                                                                                    \
        static const char *str[] = {                                                                                                                     \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4), TK_ENUM_TO_STR(t5), TK_ENUM_TO_STR(t6)}; \
        return str;                                                                                                                                      \
    }                                                                                                                                                    \
    static inline const char *toString(name e)                                                                                                           \
    {                                                                                                                                                    \
        static baseType val[] = {                                                                                                                        \
            (baseType)(v0), (baseType)(v1), (baseType)(v2), (baseType)(v3), (baseType)(v4), (baseType)(v5), (baseType)(v6)};                             \
        for (baseType i = 0; i < name##_const_size; i++)                                                                                                 \
        {                                                                                                                                                \
            if (val[i] == (baseType)e)                                                                                                                   \
                return name##_const_string()[(int)i];                                                                                                    \
        }                                                                                                                                                \
        return TK_ENUM_TO_STR_FALLBACK;                                                                                                                  \
    }

#define TK_DECLARE_ENUM_FLAGS_8(name, baseType, t0, v0, t1, v1, t2, v2, t3, v3, t4, v4, t5, v5, t6, v6, t7, v7)                                                              \
    enum class name : baseType                                                                                                                                               \
    {                                                                                                                                                                        \
        k##t0 = v0,                                                                                                                                                          \
        k##t1 = v1,                                                                                                                                                          \
        k##t2 = v2,                                                                                                                                                          \
        k##t3 = v3,                                                                                                                                                          \
        k##t4 = v4,                                                                                                                                                          \
        k##t5 = v5,                                                                                                                                                          \
        k##t6 = v6,                                                                                                                                                          \
        k##t7 = v7                                                                                                                                                           \
    };                                                                                                                                                                       \
    static const int name##_const_size = 8;                                                                                                                                  \
    static const char **name##_const_string()                                                                                                                                \
    {                                                                                                                                                                        \
        static const char *str[] = {                                                                                                                                         \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4), TK_ENUM_TO_STR(t5), TK_ENUM_TO_STR(t6), TK_ENUM_TO_STR(t7)}; \
        return str;                                                                                                                                                          \
    }                                                                                                                                                                        \
    static inline const char *toString(name e)                                                                                                                               \
    {                                                                                                                                                                        \
        static baseType val[] = {                                                                                                                                            \
            (baseType)(v0), (baseType)(v1), (baseType)(v2), (baseType)(v3), (baseType)(v4), (baseType)(v5), (baseType)(v6), (baseType)(v7)};                                 \
        for (baseType i = 0; i < name##_const_size; i++)                                                                                                                     \
        {                                                                                                                                                                    \
            if (val[i] == (baseType)e)                                                                                                                                       \
                return name##_const_string()[(int)i];                                                                                                                        \
        }                                                                                                                                                                    \
        return TK_ENUM_TO_STR_FALLBACK;                                                                                                                                      \
    }

#define TK_DECLARE_ENUM_FLAGS_9(name, baseType, t0, v0, t1, v1, t2, v2, t3, v3, t4, v4, t5, v5, t6, v6, t7, v7, t8, v8)                                                                          \
    enum class name : baseType                                                                                                                                                                   \
    {                                                                                                                                                                                            \
        k##t0 = v0,                                                                                                                                                                              \
        k##t1 = v1,                                                                                                                                                                              \
        k##t2 = v2,                                                                                                                                                                              \
        k##t3 = v3,                                                                                                                                                                              \
        k##t4 = v4,                                                                                                                                                                              \
        k##t5 = v5,                                                                                                                                                                              \
        k##t6 = v6,                                                                                                                                                                              \
        k##t7 = v7,                                                                                                                                                                              \
        k##t8 = v8                                                                                                                                                                               \
    };                                                                                                                                                                                           \
    static const int name##_const_size = 9;                                                                                                                                                      \
    static const char **name##_const_string()                                                                                                                                                    \
    {                                                                                                                                                                                            \
        static const char *str[] = {                                                                                                                                                             \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4), TK_ENUM_TO_STR(t5), TK_ENUM_TO_STR(t6), TK_ENUM_TO_STR(t7), TK_ENUM_TO_STR(t8)}; \
        return str;                                                                                                                                                                              \
    }                                                                                                                                                                                            \
    static inline const char *toString(name e)                                                                                                                                                   \
    {                                                                                                                                                                                            \
        static baseType val[] = {                                                                                                                                                                \
            (baseType)(v0), (baseType)(v1), (baseType)(v2), (baseType)(v3), (baseType)(v4), (baseType)(v5), (baseType)(v6), (baseType)(v7), (baseType)(v8)};                                     \
        for (baseType i = 0; i < name##_const_size; i++)                                                                                                                                         \
        {                                                                                                                                                                                        \
            if (val[i] == (baseType)e)                                                                                                                                                           \
                return name##_const_string()[(int)i];                                                                                                                                            \
        }                                                                                                                                                                                        \
        return TK_ENUM_TO_STR_FALLBACK;                                                                                                                                                          \
    }

#define TK_DECLARE_ENUM_FLAGS_10(name, baseType, t0, v0, t1, v1, t2, v2, t3, v3, t4, v4, t5, v5, t6, v6, t7, v7, t8, v8, t9, v9)                                                                                     \
    enum class name : baseType                                                                                                                                                                                       \
    {                                                                                                                                                                                                                \
        k##t0 = v0,                                                                                                                                                                                                  \
        k##t1 = v1,                                                                                                                                                                                                  \
        k##t2 = v2,                                                                                                                                                                                                  \
        k##t3 = v3,                                                                                                                                                                                                  \
        k##t4 = v4,                                                                                                                                                                                                  \
        k##t5 = v5,                                                                                                                                                                                                  \
        k##t6 = v6,                                                                                                                                                                                                  \
        k##t7 = v7,                                                                                                                                                                                                  \
        k##t8 = v8,                                                                                                                                                                                                  \
        k##t9 = v9                                                                                                                                                                                                   \
    };                                                                                                                                                                                                               \
    static const int name##_const_size = 10;                                                                                                                                                                         \
    static const char **name##_const_string()                                                                                                                                                                        \
    {                                                                                                                                                                                                                \
        static const char *str[] = {                                                                                                                                                                                 \
            TK_ENUM_TO_STR(t0), TK_ENUM_TO_STR(t1), TK_ENUM_TO_STR(t2), TK_ENUM_TO_STR(t3), TK_ENUM_TO_STR(t4), TK_ENUM_TO_STR(t5), TK_ENUM_TO_STR(t6), TK_ENUM_TO_STR(t7), TK_ENUM_TO_STR(t8), TK_ENUM_TO_STR(t9)}; \
        return str;                                                                                                                                                                                                  \
    }                                                                                                                                                                                                                \
    static inline const char *toString(name e)                                                                                                                                                                       \
    {                                                                                                                                                                                                                \
        static baseType val[] = {                                                                                                                                                                                    \
            (baseType)(v0), (baseType)(v1), (baseType)(v2), (baseType)(v3), (baseType)(v4), (baseType)(v5), (baseType)(v6), (baseType)(v7), (baseType)(v8), (baseType)(v9)};                                         \
        for (baseType i = 0; i < name##_const_size; i++)                                                                                                                                                             \
        {                                                                                                                                                                                                            \
            if (val[i] == (baseType)e)                                                                                                                                                                               \
                return name##_const_string()[(int)i];                                                                                                                                                                \
        }                                                                                                                                                                                                            \
        return TK_ENUM_TO_STR_FALLBACK;                                                                                                                                                                              \
    }

}