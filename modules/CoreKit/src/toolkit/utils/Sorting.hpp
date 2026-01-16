/*
 * @Author: cubevlmu khfahqp@gmail.com
 * @LastEditors: cubevlmu khfahqp@gmail.com
 * Copyright (c) 2026 by FlybirdGames, All Rights Reserved. 
 * References:
 *      https://github.com/bkaradzic/bx/blob/master/include/bx/inline/sort.inl
 */

#pragma once

#include <toolkit/common.hpp>
#include <toolkit/base/Assert.hpp>
#include <toolkit/memory/Memory.hpp>
#include <toolkit/collection/Array.hpp>

namespace toolkit {
    
    // Util for sorting data collections
    class TkSorting
    {
    public:
        // Implements stack using single linear allocation with variable capacity.
        struct Stack
        {
            static Stack& get();

            i32 count = 0;
            i32 capacity = 0;
            i32* data = nullptr;

            Stack();
            ~Stack();

            void setCapacity(i32 capacity);
            void ensureCapacity(i32 minCapacity);

            void push(const i32 item)
            {
                ensureCapacity(count + 1);
                data[count++] = item;
            }

            i32 pop()
            {
                TK_ASSERT(count > 0);
                const i32 item = data[count - 1];
                count--;
                return item;
            }
        };

    private:
        template<typename T>
        static void merge(T* data, T* tmp, i32 start, i32 mid, i32 end)
        {
            i32 h = start;
            i32 i = start;
            i32 j = mid + 1;

            while (h <= mid && j <= end)
            {
                if (data[h] < data[j])
                    tmp[i] = data[h++];
                else
                    tmp[i] = data[j++];
                i++;
            }

            if (h > mid)
            {
                for (i32 k = j; k <= end; k++)
                    tmp[i++] = data[k];
            }
            else
            {
                for (i32 k = h; k <= mid; k++)
                    tmp[i++] = data[k];
            }

            for (i32 k = start; k <= end; k++)
                data[k] = tmp[k];
        }


        template<typename T>
        static void mergeSort(T* data, T* tmp, i32 start, i32 end)
        {
            if (start >= end)
                return;
            const i32 mid = (start + end) / 2;
            mergeSort(data, tmp, start, mid);
            mergeSort(data, tmp, mid + 1, end);
            merge(data, tmp, start, mid, end);
        }

    public:
        // Sorts the linear data array using Quick Sort algorithm (non recursive version, uses temporary stack collection).
        template<typename T>
        TK_FORCE_INLINE static void quickSort(TkArray<T>& data)
        {
            quickSort(data.get(), data.count());
        }

        // Sorts the linear data array using Quick Sort algorithm (non recursive version, uses temporary stack collection).
        template<typename T>
        static void quickSort(T* data, i32 count)
        {
            if (count < 2)
                return;
            auto& stack = Stack::get();

            // Push left and right
            stack.push(0);
            stack.push(count - 1);

            // Keep sorting from stack while is not empty
            while (stack.count)
            {
                // Pop right and left
                i32 right = stack.pop();
                const i32 left = stack.pop();

                // Partition
                T* x = &data[right];
                i32 i = left - 1;
                for (i32 j = left; j <= right - 1; j++)
                {
                    if (data[j] < *x)
                    {
                        i++;
                        swap(data[i], data[j]);
                    }
                }
                swap(data[i + 1], data[right]);
                const i32 pivot = i + 1;

                // If there are elements on left side of pivot, then push left side to stack
                if (pivot - 1 > left)
                {
                    stack.push(left);
                    stack.push(pivot - 1);
                }

                // If there are elements on right side of pivot, then push right side to stack
                if (pivot + 1 < right)
                {
                    stack.push(pivot + 1);
                    stack.push(right);
                }
            }
        }


        // Sorts the linear data array using Quick Sort algorithm (non recursive version, uses temporary stack collection).
        template<typename T>
        static void quickSort(T* data, i32 count, bool (*compare)(const T& a, const T& b))
        {
            if (count < 2)
                return;
            auto& stack = Stack::get();

            // Push left and right
            stack.push(0);
            stack.push(count - 1);

            // Keep sorting from stack while is not empty
            while (stack.count)
            {
                // Pop right and left
                i32 right = stack.pop();
                const i32 left = stack.pop();

                // Partition
                T* x = &data[right];
                i32 i = left - 1;
                for (i32 j = left; j <= right - 1; j++)
                {
                    if (compare(data[j], *x))
                    {
                        i++;
                        swap(data[i], data[j]);
                    }
                }
                swap(data[i + 1], data[right]);
                const i32 pivot = i + 1;

                // If there are elements on left side of pivot, then push left side to stack
                if (pivot - 1 > left)
                {
                    stack.push(left);
                    stack.push(pivot - 1);
                }

                // If there are elements on right side of pivot, then push right side to stack
                if (pivot + 1 < right)
                {
                    stack.push(pivot + 1);
                    stack.push(right);
                }
            }
        }


        template<typename T, typename U>
        static void sortArray(T* data, i32 count, bool (*compare)(const T& a, const T& b, U* userData), U* userData)
        {
            if (count < 2)
                return;
            auto& stack = Stack::get();

            // Push left and right
            stack.push(0);
            stack.push(count - 1);

            // Keep sorting from stack while is not empty
            while (stack.count != 0)
            {
                // Pop right and left
                i32 right = stack.pop();
                const i32 left = stack.pop();

                // Partition
                T* x = &data[right];
                i32 i = left - 1;
                for (i32 j = left; j <= right - 1; j++)
                {
                    if (compare(data[j], *x, userData))
                    {
                        i++;
                        swap(data[i], data[j]);
                    }
                }
                swap(data[i + 1], data[right]);
                const i32 pivot = i + 1;

                // If there are elements on left side of pivot, then push left side to stack
                if (pivot - 1 > left)
                {
                    stack.push(left);
                    stack.push(pivot - 1);
                }

                // If there are elements on right side of pivot, then push right side to stack
                if (pivot + 1 < right)
                {
                    stack.push(pivot + 1);
                    stack.push(right);
                }
            }
        }


        // Sorts the linear data array using Quick Sort algorithm (non recursive version, uses temporary stack collection). Uses reference to values for sorting. Useful for sorting collection of pointers to objects that implement comparision operator.
        template<typename T>
        static void quickSortObj(T* data, i32 count)
        {
            if (count < 2)
                return;
            auto& stack = Stack::get();

            // Push left and right
            stack.push(0);
            stack.push(count - 1);

            // Keep sorting from stack while is not empty
            while (stack.count)
            {
                // Pop right and left
                i32 right = stack.pop();
                const i32 left = stack.pop();

                // Partition
                T x = data[right];
                i32 i = left - 1;
                for (i32 j = left; j <= right - 1; j++)
                {
                    if (*data[j] < *x)
                    {
                        i++;
                        swap(data[i], data[j]);
                    }
                }
                swap(data[i + 1], data[right]);
                const i32 pivot = i + 1;

                // If there are elements on left side of pivot, then push left side to stack
                if (pivot - 1 > left)
                {
                    stack.push(left);
                    stack.push(pivot - 1);
                }

                // If there are elements on right side of pivot, then push right side to stack
                if (pivot + 1 < right)
                {
                    stack.push(pivot + 1);
                    stack.push(right);
                }
            }
        }


        // Sorts the linear data array using Merge Sort algorithm (recursive version, uses temporary memory).
        template<typename T>
        static void mergeSort(T* data, i32 count, T* tmp = nullptr)
        {
            if (count < 2)
                return;
            const bool alloc = tmp == nullptr;
            if (alloc)
                tmp = (T*)TkMemory::allocate(TkMemory::getDefault(), sizeof(T) * count, 16);
            mergeSort(data, tmp, 0, count - 1);
            if (alloc)
                TkMemory::free(TkMemory::getDefault(), tmp);
        }


        template<typename T>
        TK_FORCE_INLINE static void mergeSort(TkArray<T>& data, TkArray<T>* tmp = nullptr)
        {
            if (tmp)
                tmp->resize(data.count());
            mergeSort(data.get(), data.count(), tmp ? tmp->get() : nullptr);
        }


        // Sorts the linear data array using Radix Sort algorithm (uses temporary keys collection).
        template<typename T, typename U>
        static void radixSort(T*& inputKeys, U*& inputValues, T* tmpKeys, U* tmpValues, i32 count)
        {
            enum
            {
                RADIXSORT_BITS = 11,
                RADIXSORT_HISTOGRAM_SIZE = 1 << RADIXSORT_BITS,
                RADIXSORT_BIT_MASK = RADIXSORT_HISTOGRAM_SIZE - 1
            };
            if (count < 2)
                return;

            T* keys = inputKeys;
            T* tempKeys = tmpKeys;
            U* values = inputValues;
            U* tempValues = tmpValues;

            u32 histogram[RADIXSORT_HISTOGRAM_SIZE];
            u16 shift = 0;
            i32 pass = 0;
            for (; pass < 6; pass++)
            {
                memset(histogram, 0, sizeof(u32) * RADIXSORT_HISTOGRAM_SIZE);

                bool sorted = true;
                T key = keys[0];
                T prevKey = key;
                for (i32 i = 0; i < count; i++)
                {
                    key = keys[i];
                    const u16 index = (key >> shift) & RADIXSORT_BIT_MASK;
                    ++histogram[index];
                    sorted &= prevKey <= key;
                    prevKey = key;
                }

                if (sorted)
                {
                    goto end;
                }

                u32 offset = 0;
                for (i32 i = 0; i < RADIXSORT_HISTOGRAM_SIZE; ++i)
                {
                    const u32 cnt = histogram[i];
                    histogram[i] = offset;
                    offset += cnt;
                }

                for (i32 i = 0; i < count; i++)
                {
                    const T k = keys[i];
                    const u16 index = (k >> shift) & RADIXSORT_BIT_MASK;
                    const u32 dest = histogram[index]++;
                    tempKeys[dest] = k;
                    tempValues[dest] = values[i];
                }

                T* const swapKeys = tempKeys;
                tempKeys = keys;
                keys = swapKeys;

                U* const swapValues = tempValues;
                tempValues = values;
                values = swapValues;

                shift += RADIXSORT_BITS;
            }

        end:
            if (pass & 1)
            {
                // Use temporary keys and values as a result
                inputKeys = tmpKeys;
                inputValues = tmpValues;
            }
        }
    };
}