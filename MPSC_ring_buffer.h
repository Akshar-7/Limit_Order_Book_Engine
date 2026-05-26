#pragma once

#include "order.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <new>
#include <immintrin.h>

// C++17 compiler does not support "hardware_destructive_interference_size"
// constexpr size_t CACHE_LINE = hardware_destructive_interference_size;
constexpr size_t CACHE_LINE = 64;

template <typename T, size_t size>
class MPSC_ring_buffer{
private:
    static_assert((size!=0) && ((size&(size-1))==0), "Capacity must be a power of 2");

    struct alignas(CACHE_LINE) Slot{
        std::atomic<bool> ready{false};
        T data;
    };

    const static size_t MASK = size-1;
    alignas(CACHE_LINE) Slot buffer[size];
    alignas(CACHE_LINE) std::atomic<size_t> head{0};
    alignas(CACHE_LINE) std::atomic<size_t> tail{0};

public:
    void push(T item){
        size_t write_idx = head.load(std::memory_order_relaxed);

        while (true){
            if (((write_idx+1)&MASK) == tail.load(std::memory_order_acquire)) {
                _mm_pause();
                continue;
            }

            if (head.compare_exchange_weak(write_idx, (write_idx+1)&MASK, std::memory_order_release, std::memory_order_relaxed))
                break;
        }

        Slot& slot = buffer[write_idx & MASK];
        slot.data = item;
        slot.ready.store(true, std::memory_order_release);
    }

    T pop(){
        size_t read_idx = tail.load(std::memory_order_relaxed);
        Slot& slot = buffer[read_idx & MASK];

        while (!slot.ready.load(std::memory_order_acquire)) {
            _mm_pause();
        }

        T item = slot.data;
        slot.ready.store(false, std::memory_order_release);

        read_idx = (read_idx+1) &MASK;
        tail.store(read_idx, std::memory_order_release);
        return item;
    }
};