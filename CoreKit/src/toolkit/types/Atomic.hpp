// Created by cubevlmu on 2025/8/4.
// Copyright (c) 2025 Flybird Games. All rights reserved.

#pragma once

#include <toolkit/globals.hpp>
#include <atomic>

namespace toolkit {

    using memOrder = std::underlying_type_t<std::memory_order>;

    enum class AtomicMemOrder : memOrder {
        kRelaxed = memOrder(std::memory_order_relaxed),
        kConsume = memOrder(std::memory_order_consume),
        kAcquire = memOrder(std::memory_order_acquire),
        kRelease = memOrder(std::memory_order_release),
        kAcqRel = memOrder(std::memory_order_acq_rel),
        kSeqCst = memOrder(std::memory_order_seq_cst)
    };

    /// Wrapper of std::atomic variable provoding util fuctions
    template<typename T, AtomicMemOrder kOrder = AtomicMemOrder::kRelaxed>
    class TkAtomic {
    public:
        static constexpr AtomicMemOrder kDefaultMemoryOrder = kOrder;

        TkAtomic() : m_val() {}

        explicit TkAtomic(T a) { setRaw(a); }

        TkAtomic(const TkAtomic &) = delete; // Non-copyable
        TkAtomic &operator=(const TkAtomic &) = delete; // Non-copyable

        void setRaw(T a) {
            *((T *) &m_val) = a;
        }

        T getRaw() const {
            return *(T *) &m_val;
        }

        T load(AtomicMemOrder ord = kDefaultMemoryOrder) const {
            return m_val.load(std::memory_order(ord));
        }

        void store(T a, AtomicMemOrder memOrd = kDefaultMemoryOrder) {
            m_val.store(a, std::memory_order(memOrd));
        }

        template<typename Y>
        T fetchAdd(Y a, AtomicMemOrder memOrd = kDefaultMemoryOrder) {
            return m_val.fetch_add(a, std::memory_order(memOrd));
        }

        template<typename Y>
        T fetchSub(Y a, AtomicMemOrder memOrd = kDefaultMemoryOrder) {
            return m_val.fetch_sub(a, std::memory_order(memOrd));
        }

        template<typename Y>
        T fetchOr(Y a, AtomicMemOrder memOrd = kDefaultMemoryOrder) {
            return m_val.fetch_or(a, std::memory_order(memOrd));
        }

        template<typename Y>
        T fetchAnd(Y a, AtomicMemOrder memOrd = kDefaultMemoryOrder) {
            return m_val.fetch_and(a, std::memory_order(memOrd));
        }

        bool compareExchange(T &expected, T desired, AtomicMemOrder successMemOrd = kDefaultMemoryOrder,
                             AtomicMemOrder failMemOrd = kDefaultMemoryOrder) {
            return m_val.compare_exchange_weak(expected, desired, std::memory_order(successMemOrd),
                                               std::memory_order(failMemOrd));
        }

        T exchange(T a, AtomicMemOrder memOrd = kDefaultMemoryOrder) {
            return m_val.exchange(a, std::memory_order(memOrd));
        }

        void min(T a) {
            T prev = load();
            while (a < prev && !compareExchange(prev, a)) {}
        }

        void max(T a) {
            T prev = load();
            while (a > prev && !compareExchange(prev, a)) {}
        }

    private:
        std::atomic<T> m_val;
    };

    // atomic for pointer values using full barrier mode (Platform dependent)
    namespace atomic {
        void* exchangePtr(void* volatile* value, void* exchange);
        bool compareExchange(volatile void **value, void *exchange, void *comperand);
        void memoryBarrier();
    }
}