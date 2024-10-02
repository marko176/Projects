#pragma once
#include <memory>
template<typename T,class Alloc = std::allocator<T>>
class vector{
    public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    T* m_data;
};