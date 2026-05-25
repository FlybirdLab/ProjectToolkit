/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 */

#pragma once

#include <toolkit/collection/Array.hpp>
#include <toolkit/base/TypeTraits.hpp>
#include <toolkit/reflect/typedef.hpp>
#include <toolkit/reflect/RkType.hpp>
#include <toolkit/reflect/invoking/ArgBuilder.hpp>
#include <toolkit/reflect/invoking/InvokeContext.hpp>
#include <toolkit/reflect/invoking/ReturnValue.hpp>

namespace toolkit
{
    class MethodCallResult
    {
    public:
        MethodCallResult() = default;
        MethodCallResult(const MethodCallResult&) = delete;
        MethodCallResult& operator=(const MethodCallResult&) = delete;
        MethodCallResult(MethodCallResult&& other) noexcept;
        MethodCallResult& operator=(MethodCallResult&& other) noexcept;
        ~MethodCallResult();

        template <class T>
        T as() const
        {
            return value.as<T>();
        }

        template <class T>
        T& ref() const
        {
            return value.ref<T>();
        }

        template <class T>
        const T& cref() const
        {
            return value.cref<T>();
        }

        template <class T>
        T* ptr() const
        {
            return value.ptr<T>();
        }

    private:
        friend class MethodCallBuilder;

        void adopt(CallFrame* frame);
        void release();

    private:
        CallFrame* m_frame = nullptr;

    public:
        ReturnValue value;
        bool ok = false;
    };

    class MethodCallBuilder
    {
    public:
        MethodCallBuilder(const MethodCallBuilder&) = delete;
        MethodCallBuilder& operator=(const MethodCallBuilder&) = delete;
        MethodCallBuilder(const RkMethod& method, void* instance);
        MethodCallBuilder(MethodCallBuilder&& other) noexcept;
        MethodCallBuilder& operator=(MethodCallBuilder&& other) noexcept;
        ~MethodCallBuilder();

        template <class T>
        MethodCallBuilder& arg(T&& value)
        {
            m_args.add(ArgBuilder::make(*m_frame, toolkit::forward<T>(value)));
            return *this;
        }

        template <class T>
        MethodCallBuilder& ref(T& value)
        {
            m_args.add(ArgBuilder::makeRef(value));
            return *this;
        }

        template <class T>
        MethodCallBuilder& cref(const T& value)
        {
            m_args.add(ArgBuilder::makeConstRef(value));
            return *this;
        }

        template <class T>
        MethodCallBuilder& ptr(T* value)
        {
            m_args.add(ArgBuilder::makePointer(*m_frame, value));
            return *this;
        }

        template <class T>
        MethodCallBuilder& move(T& value)
        {
            // Explicit transfer: after invoke(), value must be treated as moved-from.
            m_args.add(ArgBuilder::makeMove(*m_frame, value));
            return *this;
        }

        MethodCallResult invoke();

    private:
        void release();

    private:
        const RkMethod* m_method = nullptr;
        void* m_instance = nullptr;
        TkArray<ArgValue> m_args;
        CallFrame* m_frame = nullptr;
    };

    template <class T>
    struct TMethodInvokeObjectAllowed : TBoolConstant<TIsPointer<T>::Value>
    {
    };

    template <>
    struct TMethodInvokeObjectAllowed<ObjectHandle> : TBoolConstant<true>
    {
    };

    template <>
    struct TMethodInvokeObjectAllowed<RkObject*> : TBoolConstant<true>
    {
    };

    template <>
    struct TMethodInvokeObjectAllowed<const RkObject*> : TBoolConstant<true>
    {
    };

    template <class T>
    inline void* toMethodInstancePointer(T* instance)
    {
        return instance;
    }

    template <class T>
    inline void* toMethodInstancePointer(const T* instance)
    {
        return const_cast<T*>(instance);
    }

    inline void* toMethodInstancePointer(const RkObject* instance)
    {
        return const_cast<RkObject*>(instance);
    }

    template <class Ret, class Obj, class... Args>
    Ret RkMethod::invokeTyped(Obj&& object, Args&&... args) const
    {
        CallFrame frame;
        TkArray<ArgValue> argValues;
        argValues.reserve(sizeof...(Args));
        (argValues.add(ArgBuilder::make(frame, toolkit::forward<Args>(args))), ...);

        ReturnValue returnValue;
        returnValue.type = returnType;
        switch (returnKind)
        {
        case ReturnKind::kVoid:
            returnValue.kind = ReturnKind::kVoid;
            break;

        case ReturnKind::kValue:
            TK_ASSERT(returnType.isValid());
            if (returnType.isValid())
            {
                const RkType& returnTypeInfo = returnType.getType();
                returnValue.data = frame.allocate(returnTypeInfo.size, returnTypeInfo.align != 0 ? returnTypeInfo.align : 1);
            }
            returnValue.kind = ReturnKind::kValue;
            break;

        case ReturnKind::kLValueRef:
        case ReturnKind::kConstLValueRef:
        case ReturnKind::kPointer:
            returnValue.kind = returnKind;
            break;

        default:
            TK_ASSERT(false && "Unsupported ReturnKind.");
            returnValue.kind = ReturnKind::kVoid;
            break;
        }

        InvokeContext context;
        if (hasAnyFlags(flags, MethodFlags::kStatic))
        {
            context.instance = nullptr;
        }
        else if constexpr (TMethodInvokeObjectAllowed<TRemoveCVT<TRemoveReferenceT<Obj>>>::Value)
        {
            if constexpr (TIsTheSame<TRemoveCVT<TRemoveReferenceT<Obj>>, ObjectHandle>::Value)
                context.instance = resolveObjectHandle(object);
            else
                context.instance = toMethodInstancePointer(toolkit::forward<Obj>(object));
        }
        else
        {
            TK_ASSERT(false && "RkMethod::invokeTyped instance calls require ObjectHandle or object pointer.");
            context.instance = nullptr;
        }
        context.args = argValues.get();
        context.argCount = static_cast<u32>(argValues.count());
        context.result = returnValue.data;
        context.returnValue = &returnValue;

        const bool ok = invokeRaw(context);
        TK_ASSERT(ok);

        if constexpr (TIsVoidType<Ret>::Value)
        {
            return;
        }
        else if (!ok)
        {
            if constexpr (TIsLValueReference<Ret>::Value)
            {
                using ValueType = TRemoveCVT<TRemoveReferenceT<Ret>>;
                static ValueType fallback{};
                return fallback;
            }
            else
            {
                return Ret{};
            }
        }
        else if constexpr (TIsPointer<Ret>::Value)
        {
            TK_ASSERT(returnValue.kind == ReturnKind::kPointer);
            if (returnValue.kind != ReturnKind::kPointer)
                return nullptr;
            return returnValue.ptr<TRemovePointerT<Ret>>();
        }
        else if constexpr (TIsLValueReference<Ret>::Value)
        {
            using ValueType = TRemoveReferenceT<Ret>;
            if constexpr (!TAreTypesEqual<ValueType, TRemoveConstT<ValueType>>::Value)
            {
                TK_ASSERT(returnValue.kind == ReturnKind::kConstLValueRef || returnValue.kind == ReturnKind::kLValueRef);
                if (returnValue.kind != ReturnKind::kConstLValueRef && returnValue.kind != ReturnKind::kLValueRef)
                {
                    static TRemoveConstT<ValueType> fallback{};
                    return fallback;
                }
                return returnValue.cref<TRemoveConstT<ValueType>>();
            }
            else
            {
                TK_ASSERT(returnValue.kind == ReturnKind::kLValueRef);
                if (returnValue.kind != ReturnKind::kLValueRef)
                {
                    static ValueType fallback{};
                    return fallback;
                }
                return returnValue.ref<ValueType>();
            }
        }
        else
        {
            TK_ASSERT(returnValue.kind == ReturnKind::kValue || returnValue.kind == ReturnKind::kLValueRef || returnValue.kind == ReturnKind::kConstLValueRef);
            if (returnValue.kind != ReturnKind::kValue && returnValue.kind != ReturnKind::kLValueRef && returnValue.kind != ReturnKind::kConstLValueRef)
                return Ret{};
            return returnValue.as<Ret>();
        }
    }
}
