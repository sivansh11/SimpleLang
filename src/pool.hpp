#ifndef POOL_HPP
#define POOL_HPP

#include <cstdint>

namespace sl {

class pool_t {
public:
    pool_t(size_t max_size = 4 * 1024 * 1024) {
        _size = 0;
        _capacity = max_size;
        _data = new uint8_t[_capacity];
    }

    ~pool_t() {
        delete[] _data;
    }

    template <typename T, typename... Args>
    T* alloc(Args&&... args) {
        if (_size + sizeof(T) > _capacity) {
            throw std::runtime_error("no more space in pool allocator, create a larger pool");
        }
        T* val = new (_data + _size) T(std::forward<Args&&>( args )...);
        _size += sizeof(T);
        return val;
    }

private:
    uint8_t *_data;
    size_t _size;
    size_t _capacity;
};

} // namespace sl

#endif