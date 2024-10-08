#pragma once
#include <iterator>
template<typename T>
struct arrayIterator{
    using iterator_category = std::contiguous_iterator_tag;
    friend class arrayIterator<const T>;
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    
    explicit arrayIterator(pointer data) : m_data{data} {}

    arrayIterator(const arrayIterator& other) : m_data(other.m_data){

    }

    constexpr operator arrayIterator<const T>() const {
        return arrayIterator<const T>(m_data);
    }

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

    constexpr arrayIterator operator+(size_type val){
        return arrayIterator(m_data + val);
    }

    constexpr arrayIterator operator-(size_type val){
        return arrayIterator(m_data - val);
    }

    constexpr size_type operator-(arrayIterator other){
        return m_data - other.m_data; 
    }

    [[nodiscard]] constexpr bool operator==(const arrayIterator& other) const noexcept{
        return m_data == other.m_data;
    }

    [[nodiscard]] constexpr bool operator!=(const arrayIterator& other) const noexcept{
        return !(*this==other);
    }

    [[nodiscard]] constexpr bool operator<(const arrayIterator& other) const noexcept{
        return m_data<other.m_data;
    }

    [[nodiscard]] constexpr bool operator>(const arrayIterator& other) const noexcept{
        return m_data>other.m_data;
    }

    [[nodiscard]] constexpr bool operator<=(const arrayIterator& other) const noexcept{
        return m_data<=other.m_data;
    }

    [[nodiscard]] constexpr bool operator>=(const arrayIterator& other) const noexcept{
        return m_data>=other.m_data;
    }

    pointer m_data;
};