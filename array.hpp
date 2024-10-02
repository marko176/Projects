#pragma once
#include <cstdio>
#include <cstring>
#include "arrayIterator.hpp"

template <typename T, std::size_t N>
struct array{
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using iterator = arrayIterator<T>;
    using const_iterator = arrayIterator<const T>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    [[nodiscard]] constexpr reference operator[](std::size_t index){
        return m_elements[index];
    }

    [[nodiscard]] constexpr const_reference operator[](std::size_t index) const{
        return m_elements[index];
    }

    [[nodiscard]] constexpr reference at(std::size_t index){
        if(index>=N)throw std::out_of_range{};
        return m_elements[index];
    }

    [[nodiscard]] constexpr const_reference at(std::size_t index) const{
        if(index>=N)throw std::out_of_range{};
        return m_elements[index];
    }

    [[nodiscard]] constexpr reference front(){
        return m_elements[0];
    }

    [[nodiscard]] constexpr const_reference front() const{
        return m_elements[0];
    }

    [[nodiscard]] constexpr reference back(){
        return m_elements[N-1];
    }

    [[nodiscard]] constexpr const_reference back() const{
        return m_elements[N-1];
    }

    [[nodiscard]] constexpr pointer data(){
        return m_elements;
    }

    [[nodiscard]] constexpr const_pointer data() const{
        return m_elements;
    }

    [[nodiscard]] constexpr size_type size() const noexcept{
        return N;
    }

    [[nodiscard]] constexpr bool empty() const noexcept{
        return size==0;
    }

    [[nodiscard]] constexpr size_type max_size() const noexcept{
        return size();
    }

    void constexpr fill(const_reference val){
        for(reference element : m_elements){
            if constexpr(std::is_trivially_constructible_v<value_type>){
                std::memcpy(&element,&val,sizeof(val));
            }else{
                element = val;
            }
        }
    }

    void swap(array& other) noexcept(std::is_nothrow_swappable_v<value_type>){
        using std::swap;
        for(size_type i{0};i<size();++i){
            swap(operator[](i),other[i]);
        }
    }

    [[nodiscard]] constexpr iterator begin(){
        return iterator(data());
    }

    [[nodiscard]] constexpr const_iterator cbegin() const{
        return const_iterator(data());
    }

    [[nodiscard]] constexpr const_iterator begin() const{
        return cbegin();
    }

    [[nodiscard]] constexpr iterator end(){
        return iterator(data() + size());
    }

    [[nodiscard]] constexpr const_iterator cend() const{
        return const_iterator(data() + size());
    }

    [[nodiscard]] constexpr const_iterator end() const{
        return cend();
    }

    [[nodiscard]] constexpr reverse_iterator rbegin(){
        return reverse_iterator(data());
    }

    [[nodiscard]] constexpr const_reverse_iterator crbegin() const{
        return const_reverse_iterator(data());
    }

    [[nodiscard]] constexpr const_reverse_iterator rbegin() const{
        return crbegin();
    }

    [[nodiscard]] constexpr reverse_iterator rend(){
        return reverse_iterator(data() + size());
    }

    [[nodiscard]] constexpr const_reverse_iterator crend() const{
        return const_reverse_iterator(data() + size());
    }

    [[nodiscard]] constexpr const_reverse_iterator rend() const{
        return crend();
    }


    T m_elements[N];
};

