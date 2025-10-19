#pragma once

#include <cstdint>
#include <new>
#include <memory>
#include <cstddef>
#include <cstdlib>
#include <stdexcept>

constexpr size_t DEFAULT_POOL_SIZE = 1024 * 1024 * 4;

class MemoryPool {
public:
    explicit MemoryPool(size_t size = DEFAULT_POOL_SIZE) 
        : m_size(size), m_mem(nullptr), m_offset(nullptr) 
    { 
        if (m_size == 0) 
            throw std::runtime_error("memory pool size = 0");

        m_mem = static_cast<std::byte*>(std::malloc(m_size));

        if (!m_mem)
            throw std::bad_alloc();

        m_offset = m_mem;
    }
    ~MemoryPool() {
        std::free(m_mem); 
    }

    template<typename T>
    T* alloc() {
        /* 
            ONLY FOR OBJECT WITHOUT ARGS ON CONSTRUCTION! 
            DESTRUCTION IS NOT A CALLED!
        */

        void* ptr = m_offset;
        size_t space = m_size - static_cast<size_t>(m_offset - m_mem);

        if (std::align(alignof(T), sizeof(T), ptr, space) == nullptr)
            throw std::bad_alloc();

        T* tptr = reinterpret_cast<T*>(ptr);
        m_offset = reinterpret_cast<std::byte*>(reinterpret_cast<uintptr_t>(tptr) + sizeof(T));

        return new (tptr) T;
    }
private:
    std::byte* m_mem;
    std::byte* m_offset;
    size_t m_size;
};
