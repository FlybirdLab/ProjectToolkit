/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved.
 * Test target: FunctionTest.cpp
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/threading/Sync.hpp>
#include <toolkit/base/Hash.hpp>
#include <toolkit/collection/HashSet.hpp>
#include <toolkit/memory/Memory.hpp>

namespace toolkit
{
	template <typename ReturnType, typename... Params>
	class TkFunc<ReturnType(Params...)>
	{
	public:
		friend TkAction<Params...>;
		friend struct HashFunc<TkFunc<void(Params...)>>;
		typedef ReturnType (*Signature)(Params...);

	private:
		typedef ReturnType (*StubSignature)(void *, Params...);

		template <ReturnType (*Method)(Params...)>
		static ReturnType staticMethodStub(void *, Params... params)
		{
			if constexpr (std::same_as<std::remove_cv_t<ReturnType>, void>)
			{
				(Method)(toolkit::forward<Params>(params)...);
			}
			else
			{
				return (Method)(toolkit::forward<Params>(params)...);
			}
		}

		static ReturnType staticPointerMethodStub(void *callee, Params... params)
		{
			if constexpr (std::same_as<std::remove_cv_t<ReturnType>, void>)
			{
				reinterpret_cast<Signature>(callee)(toolkit::forward<Params>(params)...);
			}
			else
			{
				return reinterpret_cast<Signature>(callee)(toolkit::forward<Params>(params)...);
			}
		}

		template <class T, ReturnType (T::*method)(Params...)>
		static ReturnType classMethodStub(void *callee, Params... params)
		{
			if constexpr (std::same_as<std::remove_cv_t<ReturnType>, void>)
			{
				(reinterpret_cast<T *>(callee)->*method)(toolkit::forward<Params>(params)...);
			}
			else
			{
				return (reinterpret_cast<T *>(callee)->*method)(toolkit::forward<Params>(params)...);
			}
		}

		template <class T, ReturnType (T::*method)(Params...) const>
		static ReturnType classMethodStub(void *callee, Params... params)
		{
			if constexpr (std::same_as<std::remove_cv_t<ReturnType>, void>)
			{
				(reinterpret_cast<T *>(callee)->*method)(toolkit::forward<Params>(params)...);
			}
			else
			{
				return (reinterpret_cast<T *>(callee)->*method)(toolkit::forward<Params>(params)...);
			}
		}

		struct Lambda
		{
			i64 refs;
			void (*dtor)(void *);
		};

		void *m_callee;
		StubSignature m_func;
		Lambda *m_lambda;

		TK_FORCE_INLINE void lambdaCtor() const
		{
			TSync::interlockedIncrement((i64 volatile *)&m_lambda->refs);
		}

		TK_FORCE_INLINE void lambdaDtor()
		{
			if (TSync::interlockedDecrement(&m_lambda->refs) == 0)
			{
				((Lambda *)m_lambda)->dtor(m_callee);
				TkMemory::free(TkMemory::getDefault(), m_lambda);
			}
		}

	public:
		TkFunc()
		{
			m_callee = nullptr;
			m_func = nullptr;
			m_lambda = nullptr;
		}

		TkFunc(Signature method)
		{
			m_callee = (void *)method;
			m_func = &staticPointerMethodStub;
			m_lambda = nullptr;
		}

		template <typename T>
			requires (!std::same_as<std::remove_cvref_t<T>, TkFunc<ReturnType(Params...)>> &&
					  TInvocableAs<std::remove_cvref_t<T> &, ReturnType, Params...>)
		TkFunc(const T &lambda)
		{
			m_lambda = nullptr;
			bind<T>(lambda);
		}

		TkFunc(const TkFunc &other)
			: m_callee(other.m_callee), m_func(other.m_func), m_lambda(other.m_lambda)
		{
			if (m_lambda)
				lambdaCtor();
		}

		TkFunc(TkFunc &&other) noexcept
			: m_callee(other.m_callee), m_func(other.m_func), m_lambda(other.m_lambda)
		{
			other.m_callee = nullptr;
			other.m_func = nullptr;
			other.m_lambda = nullptr;
		}

		~TkFunc()
		{
			if (m_lambda)
				lambdaDtor();
		}

	public:
		template <ReturnType (*Method)(Params...)>
		void bind()
		{
			if (m_lambda)
				lambdaDtor();
			m_callee = nullptr;
			m_func = &staticMethodStub<Method>;
			m_lambda = nullptr;
		}

		// Binds a member TkFunc.
		template <class T, ReturnType (T::*Method)(Params...)>
		void bind(T *callee)
		{
			if (m_lambda)
				lambdaDtor();
			m_callee = callee;
			m_func = &classMethodStub<T, Method>;
			m_lambda = nullptr;
		}

		// Binds a const member TkFunc.
		template <class T, ReturnType (T::*Method)(Params...) const>
		void bind(T *callee)
		{
			if (m_lambda)
				lambdaDtor();
			m_callee = callee;
			m_func = &classMethodStub<T, Method>;
			m_lambda = nullptr;
		}

		// Binds a const member TkFunc.
		template <class T, ReturnType (T::*Method)(Params...) const>
		void bind(const T *callee)
		{
			if (m_lambda)
				lambdaDtor();
			m_callee = const_cast<T *>(callee);
			m_func = &classMethodStub<T, Method>;
			m_lambda = nullptr;
		}

		// Binds a TkFunc.
		void bind(Signature method)
		{
			if (m_lambda)
				lambdaDtor();
			m_callee = (void *)method;
			m_func = &staticPointerMethodStub;
			m_lambda = nullptr;
		}

		// Binds a lambda.
		template <typename T>
			requires (!std::same_as<std::remove_cvref_t<T>, TkFunc<ReturnType(Params...)>> &&
					  TInvocableAs<std::remove_cvref_t<T> &, ReturnType, Params...>)
		void bind(const T &lambda)
		{
			if (m_lambda)
				lambdaDtor();
			m_lambda = (Lambda *)TkMemory::allocate(TkMemory::getDefault(), sizeof(Lambda) + sizeof(T));
			m_lambda->refs = 1;
			m_lambda->dtor = [](void *callee) -> void
			{
				static_cast<T *>(callee)->~T();
			};
			m_func = [](void *callee, Params... params) -> ReturnType
			{
				if constexpr (std::same_as<std::remove_cv_t<ReturnType>, void>)
				{
					(*static_cast<T *>(callee))(toolkit::forward<Params>(params)...);
				}
				else
				{
					return (*static_cast<T *>(callee))(toolkit::forward<Params>(params)...);
				}
			};
			m_callee = (byte *)m_lambda + sizeof(Lambda);
			new (m_callee) T(lambda);
		}

		// Unbinds the TkFunc.
		void unbind()
		{
			if (m_lambda)
				lambdaDtor();
			m_callee = nullptr;
			m_func = nullptr;
			m_lambda = nullptr;
		}

	public:
		// Returns true if any TkFunc has been bound.
		TK_FORCE_INLINE bool isBinded() const
		{
			return m_func != nullptr;
		}

		// Calls the bound TkFunc (it must be assigned).
		TK_FORCE_INLINE ReturnType operator()(Params... params) const
		{
			TK_ASSERT(m_func);
			return m_func(m_callee, toolkit::forward<Params>(params)...);
		}

		TkFunc &operator=(const TkFunc &other)
		{
			if (this == &other)
				return *this;
			if (m_lambda)
				lambdaDtor();
			m_callee = other.m_callee;
			m_func = other.m_func;
			m_lambda = other.m_lambda;
			if (m_lambda)
				lambdaCtor();
			return *this;
		}

		TkFunc &operator=(TkFunc &&other) noexcept
		{
			if (this == &other)
				return *this;
			if (m_lambda)
				lambdaDtor();
			m_callee = other.m_callee;
			m_func = other.m_func;
			m_lambda = other.m_lambda;
			other.m_callee = nullptr;
			other.m_func = nullptr;
			other.m_lambda = nullptr;
			return *this;
		}

		TK_FORCE_INLINE bool operator==(const TkFunc &other) const
		{
			return m_func == other.m_func && m_callee == other.m_callee;
		}

		TK_FORCE_INLINE bool operator!=(const TkFunc &other) const
		{
			return m_func != other.m_func || m_callee != other.m_callee;
		}
	};

	// TkAction object that can be used to bind and call multiply funcs. Thread-safe to register/unregister during the call. Execution order of bound funcs is not stable.
	template <typename... Params>
	class TkAction
	{
	public:
		// Signature of the TkFunc to call.
		typedef void (*Signature)(Params...);
		// Template for the TkFunc.
		using TkFuncType = TkFunc<void(Params...)>;

	protected:
#if TkUseAtomic
		// Single allocation for list of binded funcs. Thread-safe access via atomic operations. Removing binded TkFunc simply clears the entry to handle TkFunc unregister during invocation.
		intptr volatile m_ptr = 0;
		intptr volatile m_size = 0;
		typedef void (*StubSignature)(void *, Params...);
#else
		struct Data
		{
			TkHashSet<TkFuncType> funcs;
			TkMutex mutex;
		};
		// Holds pointer to Data with funcs and mutex. Thread-safe access via atomic operations.
		intptr volatile m_data = 0;
#endif

	public:
		TkAction() = default;

		TkAction(const TkAction &other)
		{
#if TkUseAtomic
			const intptr newSize = other.m_size;
			auto newBindings = (TkFuncType *)Allocator::Allocate(newSize * sizeof(TkFuncType));
			Platform::MemoryCopy((void *)newBindings, (const void *)other.m_ptr, newSize * sizeof(TkFuncType));
			for (intptr i = 0; i < newSize; i++)
			{
				TkFuncType &f = newBindings[i];
				if (f.m_func && f.m_lambda)
					f.lambdaCtor();
			}
			m_ptr = (intptr)newBindings;
			m_size = newSize;
#else
			Data *otherData = (Data *)TSync::atomicRead(&m_data);
			if (otherData == nullptr)
				return;
			TkMutexGuard lock(otherData->mutex);
			for (auto i = otherData->funcs.Begin(); i.isNotEnd(); ++i)
				bind(i->Item);
#endif
		}

		TkAction(TkAction &&other) noexcept
		{
#if TkUseAtomic
			m_ptr = other.m_ptr;
			m_size = other.m_size;
			other.m_ptr = 0;
			other.m_size = 0;
#else
			m_data = other.m_data;
			other.m_data = 0;
#endif
		}

		~TkAction()
		{
#if TkUseAtomic
			auto ptr = (TkFuncType *)m_ptr;
			if (ptr)
			{
				while (m_size--)
				{
					if (ptr->m_lambda)
						ptr->lambdaDtor();
					++ptr;
				}
				TkMemory::free(TkMemory::getDefault(), (void *)m_ptr);
			}
#else
			Data *data = (Data *)m_data;
			if (data)
			{
				m_data = 0;
				TkMemory::deletePtr(TkMemory::getDefault(), data);
			}
#endif
		}

		TkAction &operator=(const TkAction &other)
		{
			if (this != &other)
			{
				unbindAll();
#if TkUseAtomic
				const intptr size = TSync::atomicRead((intptr volatile *)&other.m_size);
				TkFuncType *bindings = (TkFuncType *)TSync::atomicRead((intptr volatile *)&other.m_ptr);
				for (intptr i = 0; i < size; i++)
					bind(bindings[i]);
#else
				Data *otherData = (Data *)TSync::atomicRead(&m_data);
				if (otherData != nullptr)
				{
					TkMutexGuard lock(otherData->mutex);
					for (auto i = otherData->funcs.Begin(); i.isNotEnd(); ++i)
						bind(i->Item);
				}
#endif
			}
			return *this;
		}

		TkAction &operator=(TkAction &&other) noexcept
		{
			if (this != &other)
			{
#if TkUseAtomic
				m_ptr = other.m_ptr;
				m_size = other.m_size;
				other.m_ptr = 0;
				other.m_size = 0;
#else
				m_data = other.m_data;
				other.m_data = 0;
#endif
			}
			return *this;
		}

	public:
		// Binds a static TkFunc.
		template <void (*Method)(Params...)>
		void bind()
		{
			TkFuncType f;
			f.template bind<Method>();
			bind(f);
		}

		// Binds a member TkFunc.
		template <class T, void (T::*Method)(Params...)>
		void bind(T *callee)
		{
			TkFuncType f;
			f.template bind<T, Method>(callee);
			bind(f);
		}

		// Binds a const member TkFunc.
		template <class T, void (T::*Method)(Params...) const>
		void bind(T *callee)
		{
			TkFuncType f;
			f.template bind<T, Method>(callee);
			bind(f);
		}

		// Binds a const member TkFunc.
		template <class T, void (T::*Method)(Params...) const>
		void bind(const T *callee)
		{
			TkFuncType f;
			f.template bind<T, Method>(callee);
			bind(f);
		}

		// Binds a TkFunc.
		void bind(Signature method)
		{
			TkFuncType f(method);
			bind(f);
		}

		// Binds a lambda.
		template <typename T>
		void bind(const T &lambda)
		{
			TkFuncType f;
			f.template bind<T>(lambda);
			bind(f);
		}

		// Binds a TkFunc.
		void bind(const TkFuncType &f)
		{
#if TkUseAtomic
			const intptr size = TSync::atomicRead(&m_size);
			TkFuncType *bindings = (TkFuncType *)TSync::atomicRead(&m_ptr);
			if (bindings)
			{
				// Find a first free slot
				for (intptr i = 0; i < size; i++)
				{
					if (TSync::interlockedCompareExchange((intptr volatile *)&bindings[i].m_func, (intptr)f.m_func, 0) == 0)
					{
						bindings[i].m_callee = f.m_callee;
						bindings[i].m_lambda = f.m_lambda;
						if (f.m_lambda)
							f.lambdaCtor();
						return;
					}
				}
			}

			// Failed to find an empty slot in the list (empty or too small) so perform reallocation
			const intptr newSize = size ? size * 2 : 32;
			auto newBindings = (TkFuncType *)TkMemory::allocate(TkMemory::getDefault(), newSize * sizeof(TkFuncType));
			TkMemory::memoryCopy(newBindings, bindings, size * sizeof(TkFuncType));
			TkMemory::memoryClear(newBindings + size, (newSize - size) * sizeof(TkFuncType));

			// Insert TkFunc into a first slot after the old list
			newBindings[size] = f;

			// Set the new list
			auto oldBindings = (TkFuncType *)TSync::interlockedCompareExchange(&m_ptr, (intptr)newBindings, (intptr)bindings);
			if (oldBindings != bindings)
			{
				// Other thread already set the new list so free this list and try again
				TkMemory::free(TkMemory::getDefault(), newBindings);
				bind(f);
			}
			else
			{
				// Free previous bindings and update list size
				TSync::atomicStore(&m_size, newSize);
				// TODO: what is someone read this value before and is using the old table?
				TkMemory::free(TkMemory::getDefault(), bindings);
			}
#else
			Data *data = (Data *)TSync::atomicRead(&m_data);
			while (!data)
			{
				Data *newData = TkMemory::newObject<Data>(TkMemory::getDefault());
				Data *oldData = (Data *)TSync::interlockedCompareExchange(&m_data, (intptr)newData, (intptr)data);
				if (oldData != data)
				{
					// Other thread already set the new data so free it and try again
					TkMemory::free(TkMemory::getDefault(), newData);
				}
				data = (Data *)TSync::atomicRead(&m_data);
			}
			TkMutexGuard lock(data->mutex);
			data->funcs.add(f);
#endif
		}

		// Binds a static TkFunc (if not bound yet).
		template <void (*Method)(Params...)>
		void bindUnique()
		{
			TkFuncType f;
			f.template bind<Method>();
			bindUnique(f);
		}

		// Binds a member TkFunc (if not bound yet).
		template <class T, void (T::*Method)(Params...)>
		void bindUnique(T *callee)
		{
			TkFuncType f;
			f.template bind<T, Method>(callee);
			bindUnique(f);
		}

		// Binds a const member TkFunc (if not bound yet).
		template <class T, void (T::*Method)(Params...) const>
		void bindUnique(T *callee)
		{
			TkFuncType f;
			f.template bind<T, Method>(callee);
			bindUnique(f);
		}

		// Binds a const member TkFunc (if not bound yet).
		template <class T, void (T::*Method)(Params...) const>
		void bindUnique(const T *callee)
		{
			TkFuncType f;
			f.template bind<T, Method>(callee);
			bindUnique(f);
		}

		// Binds a TkFunc (if not bound yet).
		void bindUnique(Signature method)
		{
			TkFuncType f(method);
			bindUnique(f);
		}

		// Binds a TkFunc (if not bound yet).
		void bindUnique(const TkFuncType &f)
		{
			// Skip if already bound
#if TkUseAtomic
			const intptr size = TSync::atomicRead(&m_size);
			TkFuncType *bindings = (TkFuncType *)TSync::atomicRead(&m_ptr);
			if (bindings)
			{
				for (intptr i = 0; i < size; i++)
				{
					if (TSync::atomicRead((intptr volatile *)&bindings[i].m_callee) == (intptr)f.m_callee && TSync::atomicRead((intptr volatile *)&bindings[i].m_func) == (intptr)f.m_func)
						return;
				}
			}
#else
			Data *data = (Data *)TSync::atomicRead(&m_data);
			if (data)
			{
				data->mutex.lock();
				if (data->funcs.contains(f))
				{
					data->mutex.unlock();
					return;
				}
			}
#endif
			bind(f);
#if !TkUseAtomic
			if (data)
				data->mutex.unlock();
#endif
		}

		// Unbinds a static TkFunc.
		template <void (*Method)(Params...)>
		void unbind()
		{
			TkFuncType f;
			f.template bind<Method>();
			unbind(f);
		}

		// Unbinds a member TkFunc.
		template <class T, void (T::*Method)(Params...)>
		void unbind(T *callee)
		{
			TkFuncType f;
			f.template bind<T, Method>(callee);
			unbind(f);
		}

		// Unbinds a const member TkFunc.
		template <class T, void (T::*Method)(Params...) const>
		void unbind(T *callee)
		{
			TkFuncType f;
			f.template bind<T, Method>(callee);
			unbind(f);
		}

		// Unbinds a const member TkFunc.
		template <class T, void (T::*Method)(Params...) const>
		void unbind(const T *callee)
		{
			TkFuncType f;
			f.template bind<T, Method>(callee);
			unbind(f);
		}

		// Unbinds the specified TkFunc.
		void unbind(Signature method)
		{
			TkFuncType f(method);
			unbind(f);
		}

		// Unbinds the specified TkFunc.
		void unbind(const TkFuncType &f)
		{
#if TkUseAtomic
			// Find slot with that TkFunc
			const intptr size = TSync::atomicRead(&m_size);
			TkFuncType *bindings = (TkFuncType *)TSync::atomicRead(&m_ptr);
			for (intptr i = 0; i < size; i++)
			{
				if (TSync::atomicRead((intptr volatile *)&bindings[i].m_callee) == (intptr)f.m_callee && TSync::atomicRead((intptr volatile *)&bindings[i].m_func) == (intptr)f.m_func)
				{
					if (bindings[i].m_lambda)
					{
						bindings[i].lambdaDtor();
						bindings[i].m_lambda = nullptr;
					}
					TSync::atomicStore((intptr volatile *)&bindings[i].m_callee, 0);
					TSync::atomicStore((intptr volatile *)&bindings[i].m_func, 0);
					break;
				}
			}
			if ((TkFuncType *)TSync::atomicRead(&m_ptr) != bindings)
			{
				// Someone changed the bindings list so retry unbind from the new one
				unbind(f);
			}
#else
			Data *data = (Data *)TSync::atomicRead(&m_data);
			if (!data)
				return;
			TkMutexGuard lock(data->mutex);
			data->funcs.Remove(f);
#endif
		}

		// Unbinds all the funcs.
		void unbindAll()
		{
#if TkUseAtomic
			const intptr size = TSync::atomicRead(&m_size);
			TkFuncType *bindings = (TkFuncType *)TSync::atomicRead(&m_ptr);
			for (intptr i = 0; i < size; i++)
			{
				if (bindings[i].m_lambda)
				{
					bindings[i].lambdaDtor();
					bindings[i].m_lambda = nullptr;
				}
				TSync::atomicStore((intptr volatile *)&bindings[i].m_func, 0);
				TSync::atomicStore((intptr volatile *)&bindings[i].m_callee, 0);
			}
#else
			Data *data = (Data *)TSync::atomicRead(&m_data);
			if (!data)
				return;
			TkMutexGuard lock(data->mutex);
			data->funcs.clear();
#endif
		}

		// Counts the amount of bound funcs.
		i32 count() const
		{
			i32 result = 0;
#if TkUseAtomic
			const intptr size = TSync::atomicRead((intptr volatile *)&m_size);
			TkFuncType *bindings = (TkFuncType *)TSync::atomicRead((intptr volatile *)&m_ptr);
			for (intptr i = 0; i < size; i++)
			{
				if (TSync::atomicRead((intptr volatile *)&bindings[i].m_func) != 0)
					result++;
			}
#else
			Data *data = (Data *)TSync::atomicRead((intptr volatile *)&m_data);
			if (data)
			{
				TkMutexGuard lock(data->mutex);
				result = data->funcs.count();
			}
#endif
			return result;
		}

		// Gets the current capacity of TkAction table (amount of TkFunc to store before resizing).
		i32 capacity() const
		{
#if TkUseAtomic
			return (i32)TSync::atomicRead((intptr volatile *)&m_size);
#else
			i32 result = 0;
			Data *data = (Data *)TSync::atomicRead((intptr volatile *)&m_data);
			if (data)
			{
				TkMutexGuard lock(data->mutex);
				result = data->funcs.capacity();
			}
			return result;
#endif
		}

		// Determines whether any TkFunc is bound.
		bool isBinded() const
		{
#if TkUseAtomic
			const intptr size = TSync::atomicRead((intptr volatile *)&m_size);
			TkFuncType *bindings = (TkFuncType *)TSync::atomicRead((intptr volatile *)&m_ptr);
			for (intptr i = 0; i < size; i++)
			{
				if (TSync::atomicRead((intptr volatile *)&bindings[i].m_func) != 0)
					return true;
			}
			return false;
#else
			bool result = false;
			Data *data = (Data *)TSync::atomicRead((intptr volatile *)&m_data);
			if (data)
			{
				TkMutexGuard lock(data->mutex);
				result = data->funcs.count() != 0;
			}
			return result;
#endif
		}

		// Gets all the bound funcs.
		i32 getBindings(TkFuncType *buffer, i32 bufferSize) const
		{
			i32 count = 0;
#if TkUseAtomic
			const intptr size = TSync::atomicRead((intptr volatile *)&m_size);
			TkFuncType *bindings = (TkFuncType *)TSync::atomicRead((intptr volatile *)&m_ptr);
			for (intptr i = 0; i < size && i < bufferSize; i++)
			{
				buffer[count].m_func = (StubSignature)TSync::atomicRead((intptr volatile *)&bindings[i].m_func);
				if (buffer[count].m_func != nullptr)
				{
					buffer[count].m_callee = (void *)TSync::atomicRead((intptr volatile *)&bindings[i].m_callee);
					buffer[count].m_lambda = (typename TkFuncType::Lambda *)TSync::atomicRead((intptr volatile *)&bindings[i].m_lambda);
					if (buffer[count].m_lambda)
						buffer[count].lambdaCtor();
					count++;
				}
			}
#else
			Data *data = (Data *)TSync::atomicRead((intptr volatile *)&m_data);
			if (data)
			{
				TkMutexGuard lock(data->mutex);
				for (auto i = data->funcs.Begin(); i.isNotEnd(); ++i)
				{
					new (buffer + count) TkFuncType((const TkFuncType &)i->item);
					count++;
				}
			}
#endif
			return count;
		}

		// Calls all the bound funcs. Supports unbinding of the called funcs during invocation.
		void operator()(Params... params) const
		{
#if TkUseAtomic
			const intptr size = TSync::atomicRead((intptr volatile *)&m_size);
			TkFuncType *bindings = (TkFuncType *)TSync::atomicRead((intptr volatile *)&m_ptr);
			for (intptr i = 0; i < size; i++)
			{
				auto TkFunc = (StubSignature)TSync::atomicRead((intptr volatile *)&bindings->m_func);
				auto callee = (void *)TSync::atomicRead((intptr volatile *)&bindings->m_callee);
				if (TkFunc != nullptr && TkFunc == (StubSignature)TSync::atomicRead((intptr volatile *)&bindings->m_func))
					TkFunc(callee, params...);
				++bindings;
			}
#else
			Data *data = (Data *)TSync::atomicRead((intptr volatile *)&m_data);
			if (!data)
				return;
			TkMutexGuard lock(data->mutex);
			for (auto i = data->funcs.Begin(); i.isNotEnd(); ++i)
			{
				const TkFuncType &item = i->item;
				TK_ASSERT(item.m_func);
				item.m_func(item.m_callee, params...);
			}
#endif
		}
	};

	template <typename... Params>
	struct HashFunc<TkFunc<void(Params...)>>
	{
		static u32 get(const TkFunc<void(Params...)> &key)
		{
			u32 hash = HashFunc<void *>::get((void *)key.m_callee);
			combineHash(hash, HashFunc<void *>::get((void *)key.m_func));
			return hash;
		}
	};
}
