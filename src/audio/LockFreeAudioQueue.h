#pragma once
#include <atomic>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <stdexcept>

// SPSC (Single-Producer Single-Consumer) Lock-Free Ring Buffer
template<typename T>
class LockFreeAudioQueue {
private:
    std::vector<T> buffer;
    const size_t capacity_mask;
    std::atomic<size_t> write_pos;
    std::atomic<size_t> read_pos;
    
    // Aligns to cache line to prevent false sharing
    alignas(64) char padding[64];

public:
    explicit LockFreeAudioQueue(size_t size) 
        : buffer(size, T{}), 
          capacity_mask(size - 1), 
          write_pos(0), 
          read_pos(0) 
    {
        // Require power of 2 for fast modulo using bitwise AND
        if ((size & (size - 1)) != 0) {
            throw std::invalid_argument("Size must be a power of 2");
        }
    }

    bool push(const T& item) {
        size_t current_write = write_pos.load(std::memory_order_relaxed);
        size_t current_read = read_pos.load(std::memory_order_acquire);
        
        if (current_write - current_read >= buffer.size()) {
            return false; // Queue is full
        }
        
        buffer[current_write & capacity_mask] = item;
        write_pos.store(current_write + 1, std::memory_order_release);
        return true;
    }

    bool pop(T& item) {
        size_t current_read = read_pos.load(std::memory_order_relaxed);
        size_t current_write = write_pos.load(std::memory_order_acquire);
        
        if (current_read == current_write) {
            return false; // Queue is empty
        }
        
        item = buffer[current_read & capacity_mask];
        read_pos.store(current_read + 1, std::memory_order_release);
        return true;
    }
};
