/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 */

#include <toolkit/reflect/invoking/MethodCall.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/reflect/RkType.hpp>
#include <toolkit/reflect/RkManager.hpp>

namespace toolkit
{
    static bool isPassingCompatible(const ArgPassing actual, const ArgPassing expected)
    {
        switch (expected)
        {
        case ArgPassing::kValue:
            return actual == ArgPassing::kValue;
        case ArgPassing::kConstLValueRef:
            return actual == ArgPassing::kValue || actual == ArgPassing::kLValueRef || actual == ArgPassing::kConstLValueRef || actual == ArgPassing::kRValueRef;
        case ArgPassing::kLValueRef:
            return actual == ArgPassing::kLValueRef;
        case ArgPassing::kRValueRef:
            return actual == ArgPassing::kRValueRef || actual == ArgPassing::kValue;
        case ArgPassing::kPointer:
            return actual == ArgPassing::kPointer;
        default:
            return false;
        }
    }

    bool RkMethod::validateArgs(const InvokeContext &context) const
    {
        if (context.argCount != paramCount)
            return false;
        if (paramCount > 0 && context.args == nullptr)
            return false;

        for (u32 i = 0; i < paramCount; ++i)
        {
            const ArgValue &arg = context.args[static_cast<i32>(i)];
            const RkParam &param = params[static_cast<i32>(i)];
            if (arg.data == nullptr)
                return false;
            if (arg.type != param.type)
                return false;
            if (!isPassingCompatible(arg.passing, param.passing))
                return false;
        }
        return true;
    }

    bool RkMethod::invokeRaw(InvokeContext &context) const
    {
        if (invoke == nullptr)
            return false;
        if (!hasAnyFlags(flags, MethodFlags::kStatic) && context.instance == nullptr)
            return false;
        if (!validateArgs(context))
            return false;

        // TODO: When const instance invocation is added, require MethodFlags::kConst for const objects here.
        return invoke(context);
    }

    MethodCallResult::MethodCallResult(MethodCallResult &&other) noexcept
        : m_frame(other.m_frame), value(toolkit::moveTemp(other.value)), ok(other.ok)
    {
        other.m_frame = nullptr;
        other.ok = false;
    }

    MethodCallResult &MethodCallResult::operator=(MethodCallResult &&other) noexcept
    {
        if (this != &other)
        {
            release();
            m_frame = other.m_frame;
            value = toolkit::moveTemp(other.value);
            ok = other.ok;
            other.m_frame = nullptr;
            other.ok = false;
        }
        return *this;
    }

    MethodCallResult::~MethodCallResult()
    {
        release();
    }

    void MethodCallResult::adopt(CallFrame *frame)
    {
        release();
        m_frame = frame;
    }

    void MethodCallResult::release()
    {
        value.reset();
        if (m_frame != nullptr)
        {
            TkMemory::deletePtr(RkManager::getAllocator(), m_frame);
            m_frame = nullptr;
        }
    }

    MethodCallBuilder::MethodCallBuilder(const RkMethod &method, void *instance)
        : m_method(&method), m_instance(instance), m_frame(TkMemory::newObject<CallFrame>(RkManager::getAllocator()))
    {
    }

    MethodCallBuilder::MethodCallBuilder(MethodCallBuilder &&other) noexcept
        : m_method(other.m_method), m_instance(other.m_instance), m_args(toolkit::moveTemp(other.m_args)), m_frame(other.m_frame)
    {
        other.m_method = nullptr;
        other.m_instance = nullptr;
        other.m_frame = nullptr;
    }

    MethodCallBuilder &MethodCallBuilder::operator=(MethodCallBuilder &&other) noexcept
    {
        if (this != &other)
        {
            release();
            m_method = other.m_method;
            m_instance = other.m_instance;
            m_args = toolkit::moveTemp(other.m_args);
            m_frame = other.m_frame;
            other.m_method = nullptr;
            other.m_instance = nullptr;
            other.m_frame = nullptr;
        }
        return *this;
    }

    MethodCallBuilder::~MethodCallBuilder()
    {
        release();
    }

    MethodCallResult MethodCallBuilder::invoke()
    {
        MethodCallResult result;
        result.adopt(m_frame);
        m_frame = nullptr;

        if (m_method == nullptr || result.m_frame == nullptr)
            return result;

        result.value.type = m_method->returnType;
        switch (m_method->returnKind)
        {
        case ReturnKind::kVoid:
            result.value.kind = ReturnKind::kVoid;
            break;

        case ReturnKind::kValue:
            if (m_method->returnType.isValid())
            {
                const RkType &returnType = m_method->returnType.getType();
                result.value.data = result.m_frame->allocate(returnType.size, returnType.align != 0 ? returnType.align : 1);
            }
            result.value.kind = ReturnKind::kValue;
            break;

        case ReturnKind::kLValueRef:
        case ReturnKind::kConstLValueRef:
        case ReturnKind::kPointer:
            result.value.kind = m_method->returnKind;
            break;

        default:
            result.value.kind = ReturnKind::kVoid;
            break;
        }

        InvokeContext context;
        context.instance = m_instance;
        context.args = m_args.get();
        context.argCount = static_cast<u32>(m_args.count());
        context.result = result.value.data;
        context.returnValue = &result.value;

        result.ok = m_method->invokeRaw(context);
        return result;
    }

    void MethodCallBuilder::release()
    {
        if (m_frame != nullptr)
        {
            TkMemory::deletePtr(RkManager::getAllocator(), m_frame);
            m_frame = nullptr;
        }
    }

    MethodCallBuilder RkMethod::call(RkObject *instance) const
    {
        return MethodCallBuilder(*this, instance);
    }

    MethodCallBuilder RkMethod::callStatic() const
    {
        return MethodCallBuilder(*this, nullptr);
    }

    MethodCallBuilder RkMethod::call(const ObjectHandle handle) const
    {
        return MethodCallBuilder(*this, resolveObjectHandle(handle));
    }
}
