#pragma once

#include <cstddef>
#include <cstdlib>
#include <new>

constexpr size_t DEFAULT_POOL_SIZE = 1024 * 1024 * 4;

class MemoryPool {
public:
    MemoryPool(size_t size = DEFAULT_POOL_SIZE) { 
        m_mem = static_cast<unsigned char*>(malloc(m_size));
        m_offset = m_mem;
    }
    ~MemoryPool() {
        free(m_mem); 
    }

    template<typename T>
    T* alloc() {
        /* ONLY USE FOR STRUCT! ALLOC DONT CALL INIT CONSTRUCT! */
        if ((m_offset + sizeof(T)) - m_mem > m_size) {
            throw std::bad_alloc();
        }

        T* pointer = reinterpret_cast<T*>(m_offset);
        m_offset += sizeof(T);
        return pointer;
    }
private:
    unsigned char* m_mem;
    unsigned char* m_offset;
    size_t m_size;
};
