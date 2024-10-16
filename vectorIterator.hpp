#pragma once
#include <iterator>
#include <type_traits>
template <typename T>
class vectorIterator {
public:
    friend class vectorIterator<const T>;
    using iterator_category = std::contiguous_iterator_tag;
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    explicit vectorIterator(pointer data) : m_data(data) {}

    vectorIterator(const vectorIterator& other) : m_data(other.m_data) {
    }

    constexpr operator vectorIterator<const T>() const {
        return vectorIterator<const T>(m_data);
    }

    [[nodiscard]] constexpr reference operator*() {
        return *m_data;
    }
    [[nodiscard]] constexpr const_reference operator*() const {
        return *m_data;
    }

    constexpr vectorIterator& operator++() {
        ++m_data;
        return *this;
    }

    constexpr vectorIterator operator++(int) {
        vectorIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    constexpr vectorIterator& operator--() {
        --m_data;
        return *this;
    }

    constexpr vectorIterator operator--(int) {
        vectorIterator tmp = *this;
        --(*this);
        return tmp;
    }

    constexpr vectorIterator& operator+=(size_type val) {
        std::advance(m_data, val);
        return *this;
    }

    constexpr vectorIterator& operator-=(size_type val) {
        std::advance(m_data, -val);
        return *this;
    }

    constexpr vectorIterator operator+(size_type val) {
        return vectorIterator(m_data + val);
    }

    constexpr vectorIterator operator-(size_type val) {
        return vectorIterator(m_data - val);
    }

    constexpr size_type operator-(vectorIterator other) {
        return m_data - other.m_data;
    }

    [[nodiscard]] constexpr bool operator==(const vectorIterator& other) const noexcept {
        return m_data == other.m_data;
    }

    [[nodiscard]] constexpr bool operator!=(const vectorIterator& other) const noexcept {
        return !(*this == other);
    }

    [[nodiscard]] constexpr bool operator<(const vectorIterator& other) const noexcept {
        return m_data < other.m_data;
    }

    [[nodiscard]] constexpr bool operator>(const vectorIterator& other) const noexcept {
        return m_data > other.m_data;
    }

    [[nodiscard]] constexpr bool operator<=(const vectorIterator& other) const noexcept {
        return m_data <= other.m_data;
    }

    [[nodiscard]] constexpr bool operator>=(const vectorIterator& other) const noexcept {
        return m_data >= other.m_data;
    }

private:
    T* m_data;
};