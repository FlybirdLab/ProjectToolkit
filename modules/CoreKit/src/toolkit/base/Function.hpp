// Created by cubevlmu on 2025/12/8.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/threading/Sync.hpp>
#include <toolkit/base/Hash.hpp>
#include <toolkit/collection/HashSet.hpp>

namespace toolkit
{
	// The function object that supports binding static, member and lambda functions
	template <typename ReturnType, typename... Params>
	class TkFunc<ReturnType(Params...)>
	{
	public:
		friend TkAction<Params...>;
		typedef ReturnType (*Signature)(Params...);

		template <typename T, typename Enable>
		friend struct HashFunc;

	private:
		typedef ReturnType (*StubSignature)(void *, Params...);

		template <ReturnType (*Method)(Params...)>
		static ReturnType staticMethodStub(void *, Params... params)
		{
			return (Method)(forward<Params>(params)...);
		}
		static ReturnType staticPointerMethodStub(void *callee, Params... params)
		{
			return reinterpret_cast<Signature>(callee)(forward<Params>(params)...);
		}

		template <class T, ReturnType (T::*Method)(Params...)>
		static ReturnType classMethodStub(void *callee, Params... params)
		{
			return (reinterpret_cast<T *>(callee)->*Method)(forward<Params>(params)...);
		}
		template <class T, ReturnType (T::*Method)(Params...) const>
		static ReturnType classMethodStub(void *callee, Params... params)
		{
			return (reinterpret_cast<T *>(callee)->*Method)(forward<Params>(params)...);
		}

		struct Lambda
		{
			i64 Refs;
			void (*Dtor)(void *);
		};

		void *m_callee;
		StubSignature m_function;
		Lambda *m_lambda;

		TK_FORCE_INLINE void lambdaCtor() const
		{
			TSync::interlockedIncrement((i64 volatile *)&m_lambda->Refs);
		}
		TK_FORCE_INLINE void lambdaDtor()
		{
			if (TSync::interlockedDecrement(&m_lambda->Refs) == 0)
			{
				((Lambda *)m_lambda)->Dtor(m_callee);
				// TODO fix allocation DefaultPool::free(m_lambda);
				::free(m_lambda);
			}
		}

	public:
		TkFunc()
		{
			m_callee = nullptr;
			m_function = nullptr;
			m_lambda = nullptr;
		}
		TkFunc(Signature method)
		{
			m_callee = (void *)method;
			m_function = &staticPointerMethodStub;
			m_lambda = nullptr;
		}
		template <typename T>
		TkFunc(const T &lambda)
		{
			m_lambda = nullptr;
			bind<T>(lambda);
		}

		TkFunc(const TkFunc &other)
			: m_callee(other.m_callee), m_function(other.m_function), m_lambda(other.m_lambda)
		{
			if (m_lambda)
				lambdaCtor();
		}
		TkFunc(TkFunc &&other) noexcept
			: m_callee(other.m_callee), m_function(other.m_function), m_lambda(other.m_lambda)
		{
			other.m_lambda = nullptr;
		}

		~TkFunc()
		{
			if (m_lambda)
				lambdaDtor();
		}

	public:
		// Binds a static function.
		template <ReturnType (*Method)(Params...)>
		void bind()
		{
			if (m_lambda)
				lambdaDtor();
			m_callee = nullptr;
			m_function = &staticMethodStub<Method>;
			m_lambda = nullptr;
		}
		template <class T, ReturnType (T::*Method)(Params...)>
		void bind(T *callee)
		{
			if (m_lambda)
				lambdaDtor();
			m_callee = callee;
			m_function = &classMethodStub<T, Method>;
			m_lambda = nullptr;
		}
		void bind(Signature method)
		{
			if (m_lambda)
				lambdaDtor();
			m_callee = (void *)method;
			m_function = &staticPointerMethodStub;
			m_lambda = nullptr;
		}
		template <typename T>
		void bind(const T &lambda)
		{
			if (m_lambda)
				lambdaDtor();
			// TODO fix allocation m_lambda = (Lambda*)DefaultPool::allocate(sizeof(Lambda) + sizeof(T));
			m_lambda = (Lambda *)::malloc(sizeof(Lambda) + sizeof(T));

			m_lambda->Refs = 1;
			m_lambda->Dtor = [](void *callee) -> void
			{
				static_cast<T *>(callee)->~T();
			};
			m_function = [](void *callee, Params... params) -> ReturnType
			{
				return (*static_cast<T *>(callee))(forward<Params>(params)...);
			};
			m_callee = (byte *)m_lambda + sizeof(Lambda);
			new (m_callee) T(lambda);
		}

		// Unbinds the function.
		void unbind()
		{
			if (m_lambda)
				lambdaDtor();
			m_callee = nullptr;
			m_function = nullptr;
			m_lambda = nullptr;
		}

		operator Signature()
		{
			return (Signature)m_callee;
		}

	public:
		// Returns true if any function has been bound.
		TK_FORCE_INLINE bool isBinded() const
		{
			return m_function != nullptr;
		}

		TK_FORCE_INLINE ReturnType operator()(Params... params) const
		{
			TK_ASSERT(m_function);
			return m_function(m_callee, forward<Params>(params)...);
		}

		TkFunc &operator=(const TkFunc &other)
		{
			if (this == &other)
				return *this;
			m_callee = other.m_callee;
			m_function = other.m_function;
			m_lambda = other.m_lambda;
			if (m_lambda)
				lambdaCtor();
			return *this;
		}

		TkFunc &operator=(TkFunc &&other) noexcept
		{
			if (this == &other)
				return *this;
			m_callee = other.m_callee;
			m_function = other.m_function;
			m_lambda = other.m_lambda;
			other.m_callee = nullptr;
			other.m_function = nullptr;
			other.m_lambda = nullptr;
			return *this;
		}

		TK_FORCE_INLINE bool operator==(const TkFunc &other) const
		{
			return m_function == other.m_function && m_callee == other.m_callee;
		}

		TK_FORCE_INLINE bool operator!=(const TkFunc &other) const
		{
			return m_function != other.m_function || m_callee != other.m_callee;
		}
	};

	// TkAction object that can be used to bind and call multiply functions. Thread-safe to register/unregister during the call. Execution order of bound functions is not stable
	template <typename... Params>
	class TkAction
	{
	public:
		// Signature of the function to call.
		typedef void (*Signature)(Params...);
		using TkFuncType = TkFunc<void(Params...)>;

	protected:
		struct Data
		{
			TkHashSet<TkFuncType> funcs;
			TkMutex locker;
		};
		// Holds pointer to Data with funcs and locker. Thread-safe access via atomic operations.
		intptr_t volatile m_data = 0;
		IAllocator &m_alloc;

	public:
		TkAction(IAllocator &alloc = TkMemory::getDefault())
			: m_alloc{alloc}
		{
		}

		TkAction(const TkAction &other)
			: m_alloc{other.m_alloc}
		{
			Data *otherData = (Data *)TSync::atomicRead(&m_data);
			if (otherData == nullptr)
				return;
			TkMutexGuard lock(otherData->locker);
			for (auto i = otherData->funcs.Begin(); i.IsNotEnd(); ++i)
				bind(i->Item);
		}

		TkAction(TkAction &&other) noexcept
		{
			m_alloc = other.m_alloc; // TODO
			m_data = other.m_data;
			other.m_data = 0;
		}

		~TkAction()
		{
			Data *data = (Data *)m_data;
			if (data)
			{
				m_data = 0;
				TkMemory::deletePtr(m_alloc, data);
			}
		}

		TkAction &operator=(const TkAction &other)
		{
			if (this != &other)
			{
				unbindAll();
				Data *otherData = (Data *)TSync::atomicRead(&m_data);
				if (otherData != nullptr)
				{
					TkMutexGuard lock(otherData->locker);
					for (auto i = otherData->funcs.Begin(); i.IsNotEnd(); ++i)
						bind(i->Item);
				}
			}
			return *this;
		}

		TkAction &operator=(TkAction &&other) noexcept
		{
			if (this != &other)
			{
				m_data = other.m_data;
				other.m_data = 0;
			}
			return *this;
		}

	public:
		template <void (*Method)(Params...)>
		void bind()
		{
			TkFuncType f;
			f.template bind<Method>();
			bind(f);
		}
		template <class T, void (T::*Method)(Params...)>
		void bind(T *callee)
		{
			TkFuncType f;
			f.template bind<T, Method>(callee);
			bind(f);
		}
		void bind(Signature method)
		{
			TkFuncType f(method);
			bind(f);
		}
		template <typename T>
		void bind(const T &lambda)
		{
			TkFuncType f;
			f.template bind<T>(lambda);
			bind(f);
		}
		void bind(const TkFuncType &f)
		{
			Data *data = (Data *)TSync::atomicRead(&m_data);
			while (!data)
			{
				Data *newData = TkMemory::newObject<Data>(m_alloc);
				Data *oldData = (Data *)TSync::interlockedCompareExchange(&m_data, (intptr)newData, (intptr)data);
				if (oldData != data)
				{
					// Other thread already set the new data so free it and try again
					TkMemory::deletePtr(m_alloc, newData);
				}
				data = (Data *)TSync::atomicRead(&m_data);
			}
			TkMutexGuard lock(data->locker);
			data->funcs.add(f);
		}

		// Binds a static function (if not bound yet).
		template <void (*Method)(Params...)>
		void bindUnique()
		{
			TkFuncType f;
			f.template bind<Method>();
			bindUnique(f);
		}

		// Binds a member function (if not bound yet).
		template <class T, void (T::*Method)(Params...)>
		void bindUnique(T *callee)
		{
			TkFuncType f;
			f.template bind<T, Method>(callee);
			bindUnique(f);
		}

		// Binds a function (if not bound yet).
		void bindUnique(Signature method)
		{
			TkFuncType f(method);
			bindUnique(f);
		}

		// Binds a function (if not bound yet).
		void bindUnique(const TkFuncType &f)
		{
			Data *data = (Data *)TSync::atomicRead(&m_data);
			if (data)
			{
				data->locker.lock();
				if (data->funcs.contains(f))
				{
					data->locker.unlock();
					return;
				}
			}
			bind(f);
			if (data)
				data->locker.unlock();
		}

		template <void (*Method)(Params...)>
		void unbind()
		{
			TkFuncType f;
			f.template bind<Method>();
			unbind(f);
		}
		template <class T, void (T::*Method)(Params...)>
		void unbind(T *callee)
		{
			TkFuncType f;
			f.template bind<T, Method>(callee);
			unbind(f);
		}
		void unbind(Signature method)
		{
			TkFuncType f(method);
			unbind(f);
		}
		void unbind(const TkFuncType &f)
		{
			Data *data = (Data *)TSync::atomicRead(&m_data);
			if (!data)
				return;
			TkMutexGuard lock(data->locker);
			data->funcs.remove(f);
		}

		// Unbinds all the functions.
		void unbindAll()
		{
			Data *data = (Data *)TSync::atomicRead(&m_data);
			if (!data)
				return;
			TkMutexGuard lock(data->locker);
			data->funcs.clear();
		}

		// Counts the amount of bound functions.
		i32 count() const
		{
			i32 result = 0;
			Data *data = (Data *)TSync::atomicRead((intptr volatile *)&m_data);
			if (data)
			{
				TkMutexGuard lock(data->locker);
				result = data->funcs.count();
			}
			return result;
		}

		// Gets the current capacity of delegate table (amount of function to store before resizing).
		i32 capacity() const
		{
			i32 result = 0;
			Data *data = (Data *)TSync::atomicRead((intptr volatile *)&m_data);
			if (data)
			{
				TkMutexGuard lock(data->locker);
				result = data->funcs.capacity();
			}
			return result;
		}

		// Determines whether any function is bound.
		bool isBinded() const
		{
			bool result = false;
			Data *data = (Data *)TSync::atomicRead((intptr volatile *)&m_data);
			if (data)
			{
				TkMutexGuard lock(data->locker);
				result = data->funcs.count() != 0;
			}
			return result;
		}

		// Gets all the bound functions.
		i32 getBindings(TkFuncType *buffer, i32 bufferSize) const
		{
			i32 count = 0;
			Data *data = (Data *)TSync::atomicRead((intptr volatile *)&m_data);
			if (data)
			{
				TkMutexGuard lock(data->locker);
				for (auto i = data->funcs.Begin(); i.isNotEnd(); ++i)
				{
					new (buffer + count) TkFuncType((const TkFuncType &)i->item);
					count++;
				}
			}
			return count;
		}

		// Calls all the bound functions. Supports unbinding of the called functions during invocation.
		void operator()(Params... params) const
		{
			Data *data = (Data *)TSync::atomicRead((intptr volatile *)&m_data);
			if (!data)
				return;
			TkMutexGuard lock(data->locker);
			for (auto i = data->funcs.Begin(); i.isNotEnd(); ++i)
			{
				const TkFuncType &item = i->item;
				TK_ASSERT(item.m_function);
				item.m_function(item.m_callee, params...);
			}
		}
	};

	template <typename... Params>
	struct HashFunc<TkFunc<void(Params...)>>
	{
		static u32 get(const TkFunc<void(Params...)> &key)
		{
			u32 hash = HashFunc<void *>::get((void *)key.m_callee);
			combineHash(hash, HashFunc<void *>::get((void *)key.m_function));
			return hash;
		}
	};

}