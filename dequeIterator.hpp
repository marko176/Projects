#pragma once
#include <iterator>

template <typename T>
class dequeIterator{
    public:
    using iterator_category = std::random_access_iterator_tag;
    using reference = T&;
    using const_reference = const T&;
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    static constexpr difference_type block_size = sizeof(T) <= 4 ? 16 : 8;

    dequeIterator(T** data,size_type pos) : mData(data) , mPos(pos){

    }

    operator dequeIterator<const T>() {
        return dequeIterator<const T>(const_cast<const T**>(mData),mPos);
    }

    constexpr dequeIterator& operator++(){
        ++mPos;
        return *this;
    }

    constexpr dequeIterator& operator--(){
        --mPos;
        return *this;
    }

    constexpr dequeIterator& operator++(int){
        dequeIterator temp = *this; 
        ++(*this);
        return temp;
    }

    constexpr dequeIterator& operator--(int){
        dequeIterator temp = *this; 
        --(*this);
        return temp;
    }

    [[nodiscard]] constexpr bool operator==(const dequeIterator& other) const noexcept{
        return mData == other.mData && mPos==other.mPos;
    }

    [[nodiscard]] constexpr bool operator!=(const dequeIterator& other) const noexcept{
        return !(*this==other);
    }

    constexpr reference operator*(){
        return mData[mPos/block_size][mPos%block_size];
    }

    [[nodiscard]] constexpr size_type operator-(const dequeIterator& other) const {
        return mPos-other.mPos;
    }

    [[nodiscard]] constexpr size_type operator+(const dequeIterator& other) const {
        return mPos+other.mPos;
    }

    [[nodiscard]] constexpr dequeIterator operator-(size_type count) const {
        return dequeIterator(mData,mPos+count);
    }

    [[nodiscard]] constexpr dequeIterator operator+(size_type count) const {
        return dequeIterator(mData,mPos+count);
    }

    private:
    T** mData;
    size_type mPos;
};