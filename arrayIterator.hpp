#pragma once

template<typename T>
struct arrayIterator{
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    
    arrayIterator(pointer data) : m_data{data} {}

    [[nodiscard]] constexpr reference operator*(){
        return *m_data;
    }
    [[nodiscard]] constexpr const_reference operator*() const{
        return *m_data;
    }

    constexpr arrayIterator& operator++(){
        ++m_data;
        return *this;
    }

    constexpr arrayIterator operator++(int){
        arrayIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    constexpr arrayIterator& operator--(){
        --m_data;
        return *this;
    }

    constexpr arrayIterator operator--(int){
        arrayIterator tmp = *this;
        --(*this);
        return tmp;
    }

    constexpr arrayIterator& operator+=(size_type val){
        std::advance(m_data,val);
        return *this;
    }

    constexpr arrayIterator& operator-=(size_type val){
        std::advance(m_data,-val);
        return *this;
    }

    [[nodiscard]] constexpr bool operator==(const arrayIterator& other) const noexcept{
        return m_data == other.m_data;
    }

    [[nodiscard]] constexpr bool operator!=(const arrayIterator& other) const noexcept{
        return !(*this==other);
    }

    pointer m_data;
};