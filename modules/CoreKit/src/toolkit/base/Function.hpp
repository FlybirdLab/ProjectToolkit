// Created by cubevlmu on 2025/12/8.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/threading/Sync.hpp>
#include <toolkit/base/Hash.hpp>
#include <toolkit/collection/HashSet.hpp>

namespace toolkit {

	// The function object that supports binding static, member and lambda functions
	template<typename ReturnType, typename... Params>
	class TFunc<ReturnType(Params...)>
	{
	public:
		friend TAction<Params...>;
		typedef ReturnType(*Signature)(Params...);

		template<typename... TParams>
		friend u32 getHash(const TFunc<void(TParams...)>& key);

	private:
		typedef ReturnType(*StubSignature)(void*, Params...);

		template<ReturnType(*Method)(Params...)>
		static ReturnType staticMethodStub(void*, Params... params)
		{
			return (Method)(forward<Params>(params)...);
		}
		static ReturnType staticPointerMethodStub(void* callee, Params... params)
		{
			return reinterpret_cast<Signature>(callee)(forward<Params>(params)...);
		}

		template<class T, ReturnType(T::* Method)(Params...)>
		static ReturnType classMethodStub(void* callee, Params... params)
		{
			return (reinterpret_cast<T*>(callee)->*Method)(forward<Params>(params)...);
		}
		template<class T, ReturnType(T::* Method)(Params...) const>
		static ReturnType classMethodStub(void* callee, Params... params)
		{
			return (reinterpret_cast<T*>(callee)->*Method)(forward<Params>(params)...);
		}

		struct Lambda
		{
			i64 Refs;
			void (*Dtor)(void*);
		};

		void* m_callee;
		StubSignature m_function;
		Lambda* m_lambda;

		TK_FORCE_INLINE void lambdaCtor() const
		{
			TSync::interlockedIncrement((i64 volatile*)&m_lambda->Refs);
		}
		TK_FORCE_INLINE void lambdaDtor()
		{
			if (TSync::interlockedDecrement(&m_lambda->Refs) == 0)
			{
				((Lambda*)m_lambda)->Dtor(m_callee);
				//TODO fix allocation DefaultPool::free(m_lambda);
				::free(m_lambda);
			}
		}

	public:
		TFunc()
		{
			m_callee = nullptr;
			m_function = nullptr;
			m_lambda = nullptr;
		}
		TFunc(Signature method)
		{
			m_callee = (void*)method;
			m_function = &staticPointerMethodStub;
			m_lambda = nullptr;
		}
		template<typename T>
		TFunc(const T& lambda)
		{
			m_lambda = nullptr;
			bind<T>(lambda);
		}

		TFunc(const TFunc& other)
			: m_callee(other.m_callee)
			, m_function(other.m_function)
			, m_lambda(other.m_lambda)
		{
			if (m_lambda)
				lambdaCtor();
		}
		TFunc(TFunc&& other) noexcept
			: m_callee(other.m_callee)
			, m_function(other.m_function)
			, m_lambda(other.m_lambda)
		{
			other.m_lambda = nullptr;
		}

		~TFunc()
		{
			if (m_lambda)
				lambdaDtor();
		}

	public:
		// Binds a static function.
		template<ReturnType(*Method)(Params...)>
		void bind()
		{
			if (m_lambda)
				lambdaDtor();
			m_callee = nullptr;
			m_function = &staticMethodStub<Method>;
			m_lambda = nullptr;
		}
		template<class T, ReturnType(T::* Method)(Params...)>
		void bind(T* callee)
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
			m_callee = (void*)method;
			m_function = &staticPointerMethodStub;
			m_lambda = nullptr;
		}
		template<typename T>
		void bind(const T& lambda)
		{
			if (m_lambda)
				lambdaDtor();
			//TODO fix allocation m_lambda = (Lambda*)DefaultPool::allocate(sizeof(Lambda) + sizeof(T));
			m_lambda = (Lambda*)::malloc(sizeof(Lambda) + sizeof(T));

			m_lambda->Refs = 1;
			m_lambda->Dtor = [](void* callee) -> void
			{
				static_cast<T*>(callee)->~T();
			};
			m_function = [](void* callee, Params... params) -> ReturnType
			{
				return (*static_cast<T*>(callee))(forward<Params>(params)...);
			};
			m_callee = (byte*)m_lambda + sizeof(Lambda);
			new(m_callee) T(lambda);
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

		operator Signature() {
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
			ASSERT_LOW_LAYER(m_function);
			return m_function(m_callee, forward<Params>(params)...);
		}

		TFunc& operator=(const TFunc& other)
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

		TFunc& operator=(TFunc&& other) noexcept
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

		TK_FORCE_INLINE bool operator==(const TFunc& other) const
		{
			return m_function == other.m_function && m_callee == other.m_callee;
		}

		TK_FORCE_INLINE bool operator!=(const TFunc& other) const
		{
			return m_function != other.m_function || m_callee != other.m_callee;
		}
	};


	// TAction object that can be used to bind and call multiply functions. Thread-safe to register/unregister during the call. Execution order of bound functions is not stable
	template<typename... Params>
	class TAction
	{
	public:
		// Signature of the function to call.
		typedef void (*Signature)(Params...);
		using TFuncType = TFunc<void(Params...)>;

	protected:
		struct Data
		{
			THashSet<TFuncType> TFuncs;
			TkMutex Locker;
		};
		// Holds pointer to Data with TFuncs and Locker. Thread-safe access via atomic operations.
		intptr_t volatile m_data = 0;

	public:
		TAction()
		{
		}

		TAction(const TAction& other)
		{
			Data* otherData = (Data*)TSync::atomicRead(&m_data);
			if (otherData == nullptr)
				return;
			TkMutexGuard lock(otherData->Locker);
			for (auto i = otherData->TFuncs.Begin(); i.IsNotEnd(); ++i)
				bind(i->Item);
		}


		TAction(TAction&& other) noexcept
		{
			m_data = other.m_data;
			other.m_data = 0;
		}


		~TAction()
		{
			Data* data = (Data*)m_data;
			if (data)
			{
				m_data = 0;
				deletePtr(data);
			}
		}

		TAction& operator=(const TAction& other)
		{
			if (this != &other)
			{
				unbindAll();
				Data* otherData = (Data*)TSync::atomicRead(&m_data);
				if (otherData != nullptr)
				{
					TkMutexGuard lock(otherData->Locker);
					for (auto i = otherData->TFuncs.Begin(); i.IsNotEnd(); ++i)
						bind(i->Item);
				}
			}
			return *this;
		}

		TAction& operator=(TAction&& other) noexcept
		{
			if (this != &other)
			{
				m_data = other.m_data;
				other.m_data = 0;
			}
			return *this;
		}

	public:
		template<void(*Method)(Params...)>
		void bind()
		{
			TFuncType f;
			f.template bind<Method>();
			bind(f);
		}
		template<class T, void(T::* Method)(Params...)>
		void bind(T* callee)
		{
			TFuncType f;
			f.template bind<T, Method>(callee);
			bind(f);
		}
		void bind(Signature method)
		{
			TFuncType f(method);
			bind(f);
		}
		template<typename T>
		void bind(const T& lambda)
		{
			TFuncType f;
			f.template bind<T>(lambda);
			bind(f);
		}
		void bind(const TFuncType& f)
		{
			Data* data = (Data*)TSync::atomicRead(&m_data);
			while (!data)
			{
				Data* newData = newObject<Data>();
				Data* oldData = (Data*)TSync::interlockedCompareExchange(&m_data, (intptr)newData, (intptr)data);
				if (oldData != data)
				{
					// Other thread already set the new data so free it and try again
					deletePtr(newData);
				}
				data = (Data*)TSync::atomicRead(&m_data);
			}
			TkMutexGuard lock(data->Locker);
			data->TFuncs.add(f);
		}

		// Binds a static function (if not bound yet).
		template<void(*Method)(Params...)>
		void bindUnique()
		{
			TFuncType f;
			f.template bind<Method>();
			bindUnique(f);
		}

		// Binds a member function (if not bound yet).
		template<class T, void(T::* Method)(Params...)>
		void bindUnique(T* callee)
		{
			TFuncType f;
			f.template bind<T, Method>(callee);
			bindUnique(f);
		}

		// Binds a function (if not bound yet).
		void bindUnique(Signature method)
		{
			TFuncType f(method);
			bindUnique(f);
		}

		// Binds a function (if not bound yet).
		void bindUnique(const TFuncType& f)
		{
			Data* data = (Data*)TSync::atomicRead(&m_data);
			if (data)
			{
				data->Locker.lock();
				if (data->TFuncs.contains(f))
				{
					data->Locker.unlock();
					return;
				}
			}
			bind(f);
			if (data)
				data->Locker.unlock();
		}


		template<void(*Method)(Params...)>
		void unbind()
		{
			TFuncType f;
			f.template bind<Method>();
			unbind(f);
		}
		template<class T, void(T::* Method)(Params...)>
		void unbind(T* callee)
		{
			TFuncType f;
			f.template bind<T, Method>(callee);
			unbind(f);
		}
		void unbind(Signature method)
		{
			TFuncType f(method);
			unbind(f);
		}
		void unbind(const TFuncType& f)
		{
			Data* data = (Data*)TSync::atomicRead(&m_data);
			if (!data)
				return;
			TkMutexGuard lock(data->Locker);
			data->TFuncs.remove(f);
		}

		// Unbinds all the functions.
		void unbindAll()
		{
			Data* data = (Data*)TSync::atomicRead(&m_data);
			if (!data)
				return;
			TkMutexGuard lock(data->Locker);
			data->TFuncs.clear();
		}

		// Counts the amount of bound functions.
		i32 count() const
		{
			i32 result = 0;
			Data* data = (Data*)TSync::atomicRead((intptr volatile*)&m_data);
			if (data)
			{
				TkMutexGuard lock(data->Locker);
				result = data->TFuncs.count();
			}
			return result;
		}

		// Gets the current capacity of delegate table (amount of function to store before resizing).
		i32 capacity() const
		{
			i32 result = 0;
			Data* data = (Data*)TSync::atomicRead((intptr volatile*)&m_data);
			if (data)
			{
				TkMutexGuard lock(data->Locker);
				result = data->TFuncs.capacity();
			}
			return result;
		}

		// Determines whether any function is bound.
		bool isBinded() const
		{
			bool result = false;
			Data* data = (Data*)TSync::atomicRead((intptr volatile*)&m_data);
			if (data)
			{
				TkMutexGuard lock(data->Locker);
				result = data->TFuncs.count() != 0;
			}
			return result;
		}

		// Gets all the bound functions.
		i32 getBindings(TFuncType* buffer, i32 bufferSize) const
		{
			i32 count = 0;
			Data* data = (Data*)TSync::atomicRead((intptr volatile*)&m_data);
			if (data)
			{
				TkMutexGuard lock(data->Locker);
				for (auto i = data->TFuncs.Begin(); i.isNotEnd(); ++i)
				{
					new(buffer + count) TFuncType((const TFuncType&)i->Item);
					count++;
				}
			}
			return count;
		}

		// Calls all the bound functions. Supports unbinding of the called functions during invocation.
		void operator()(Params... params) const
		{
			Data* data = (Data*)TSync::atomicRead((intptr volatile*)&m_data);
			if (!data)
				return;
			TkMutexGuard lock(data->Locker);
			for (auto i = data->TFuncs.Begin(); i.isNotEnd(); ++i)
			{
				const TFuncType& item = i->Item;
				ASSERT_LOW_LAYER(item.m_function);
				item.m_function(item.m_callee, params...);
			}
		}
	};

	template<typename... Params>
	inline u32 getHash(const TFunc<void(Params...)>& key)
	{
		u32 hash = getHash((void*)key.m_callee);
		combineHash(hash, getHash((void*)key.m_function));
		return hash;
	}
	
	// Action delegate
	typedef TAction<> Action;
}