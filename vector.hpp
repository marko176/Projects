#pragma once
#include "vectorIterator.hpp"
#include <algorithm>
#include <bit>
#include <memory>
#include <utility>

template <typename T, class Alloc = std::allocator<T>>
class vector {
public:
    template <typename U>
    friend class vectorIterator;
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using allocator_type = Alloc;
    using iterator = vectorIterator<T>;
    using const_iterator = vectorIterator<const T>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    vector() noexcept(noexcept(allocator_type{})) : m_data(nullptr), m_size(0), m_capacity(0), m_alloc{allocator_type{}} {}

    explicit vector(const allocator_type& alloc) noexcept : m_data(nullptr), m_size(0), m_capacity(0), m_alloc{alloc} {}

    explicit vector(size_type size, const_reference val = value_type{}, const allocator_type& alloc = allocator_type{}) : m_data(change_capacity(std::bit_ceil(size))), m_size(0), m_capacity(std::bit_ceil(size)), m_alloc{alloc} {
        assign(size, val);
    }

    vector(std::initializer_list<value_type> list, const allocator_type& alloc = allocator_type{}) : vector(alloc) {
        for (const_reference val : list)
            push_back(val);
    }

    template <typename It>
    vector(It first, It last, const allocator_type& alloc = allocator_type{}) : vector(alloc) {
        if constexpr (std::is_same_v<std::contiguous_iterator_tag, typename std::iterator_traits<It>::iterator_category>) {
            reserve(last - first);
        }
        while (first != last)
            push_back(*first++);
    }

    vector(const vector& other) : vector(other.cbegin(), other.cend(), other.get_allocator()) {
        // copy constructor
    }

    vector(vector&& other) : m_data(std::exchange(other.data(), nullptr)), m_size(std::exchange(other.size(), 0)), m_capacity(std::exchange(other.capacity(), 0)), m_alloc(other.get_allocator()) {
        // move constructor
    }

    vector& operator=(const vector& other) {
        // copy & swap
        vector temp = other;
        swap(temp);
        return *this;
    }

    vector& operator=(vector&& other) noexcept {
        m_data = std::exchange(other.data(), nullptr);
        m_size = std::exchange(other.size(), 0);
        m_capacity = std::exchange(other.capacity(), 0);
        m_alloc = other.get_allocator();
        return *this;
    }

    vector& operator=(std::initializer_list<value_type> list) {
        assign(list);
        return *this;
    }

    ~vector() noexcept {
        delete_data(m_size);
        free_data(m_data);
    }

    void constexpr push_back(const_reference val) {
        if (size() >= capacity())
            increase_capacity(capacity() * 2);
        std::construct_at(&m_data[m_size++], val);
    }

    void constexpr push_back(value_type&& val) {
        if (size() >= capacity())
            increase_capacity(capacity() * 2);
        std::construct_at(&m_data[m_size++], std::move(val));
    }

    template <typename... Args>
    reference constexpr emplace_back(Args&&... args) {
        if (size() >= capacity())
            increase_capacity(capacity() * 2);
        return std::construct_at(&m_data[m_size++], std::forward<Args>(args)...);
    }

    constexpr void clear() noexcept(noexcept(~value_type())) {
        delete_data(size());
        m_size = 0;
    }

    constexpr void pop_back() {
        // std::allocator_traits<allocator_type>::template destroy<int>(m_alloc,&m_data[--m_size]);
        //  if we had template destroy it wouldnt work
        std::allocator_traits<allocator_type>::destroy(m_alloc, &m_data[--m_size]);
    }

    constexpr void assign(size_type count, const_reference val) {
        clear();
        reserve(count);
        for (size_type i{0}; i < count; ++i)
            push_back(val);
    }

    template <typename It>
    constexpr void assign(It first, It last) {
        clear();
        if constexpr (std::is_same_v<std::contiguous_iterator_tag, typename std::iterator_traits<It>::iterator_category>) {
            reserve(std::distance(first, last));
        }
        while (first != last)
            push_back(*first++);
    }

    constexpr void assign(std::initializer_list<value_type> list) {
        clear();
        for (const_reference val : list)
            push_back(val);
    }

    template <typename... Args>
    constexpr iterator emplace(const_iterator pos, Args&&... args) {
        size_type index = pos - begin();
        emplace_back(std::forward<Args>(args)...);
        partition(index, 1);
        return begin() + index;
    }

    constexpr iterator insert(const_iterator pos, const_reference val) {
        return insert(pos, static_cast<size_type>(1), val);
    }

    constexpr iterator insert(const_iterator pos, value_type&& val) {
        size_type index = pos - begin();
        push_back(std::move(val));
        partition(index, 1);
        return begin() + index;
    }

    constexpr iterator insert(const_iterator pos, size_type count, const_reference val) {
        size_type index = pos - begin();
        for (size_type i = 0; i < count; ++i)
            push_back(val);
        partition(index, count);
        return begin() + index;
    }

    template <typename It>
    constexpr iterator insert(const_iterator pos, It first, It last) {
        size_type index = pos - begin();
        size_type count = 0;
        while (first != last) {
            push_back(*first++);
            ++count;
        }
        partition(index, count);
        return begin() + index;
    }

    constexpr iterator insert(const_iterator pos, std::initializer_list<T> list) {
        size_type index = pos - begin();
        size_type count = 0;
        for (const_reference n : list) {
            push_back(n);
            ++count;
        }
        partition(index, count);
        return begin() + index;
    }

    iterator constexpr erase(iterator pos) {
        return erase(pos, std::next(pos));
    }

    iterator constexpr erase(iterator first, iterator last) {
        if (first == last)
            return first;
        iterator tmp = first;
        while (last != end()) {
            if constexpr (std::is_trivially_copyable_v<value_type>) {
                memmove(&(*first), &(*last), (end() - last) * sizeof(value_type));
                first += end() - last;
                break;
            } else if constexpr (std::is_nothrow_constructible_v<value_type>) {
                *first++ = std::move(*last++);
            } else {
                *first++ = *last++;
            }
        }

        while (first != end())
            pop_back();
        return tmp;
    }

    void reserve(size_type size) {
        increase_capacity(std::bit_ceil(size));
    }

    void resize(size_type count, const_reference val = value_type{}) {
        while (size() > count)
            pop_back();
        while (size() < count)
            push_back(val);
    }

    [[nodiscard]] allocator_type get_allocator() const {
        return m_alloc;
    }

    [[nodiscard]] constexpr reference operator[](size_type index) {
        return m_data[index];
    }
    [[nodiscard]] constexpr const_reference operator[](size_type index) const {
        return m_data[index];
    }

    [[nodiscard]] constexpr reference at(size_type index) {
        if (index >= m_size)
            throw std::out_of_range{"Vector index out of range!"};
        return m_data[index];
    }
    [[nodiscard]] constexpr const_reference at(size_type index) const {
        if (index >= m_size)
            throw std::out_of_range{"Vector index out of range!"};
        return m_data[index];
    }

    [[nodiscard]] constexpr reference front() {
        return m_data[0];
    }
    [[nodiscard]] constexpr const_reference front() const {
        return m_data[0];
    }

    [[nodiscard]] constexpr reference back() {
        return m_data[size() - 1];
    }
    [[nodiscard]] constexpr const_reference back() const {
        return m_data[size() - 1];
    }

    [[nodiscard]] constexpr size_type size() const noexcept {
        return m_size;
    }

    [[nodiscard]] constexpr size_type max_size() const noexcept {
        return UINT64_MAX; //?
    }

    [[nodiscard]] constexpr size_type capacity() const noexcept {
        return m_capacity;
    }

    [[nodiscard]] constexpr bool empty() const noexcept {
        return size() == 0;
    }

    [[nodiscard]] constexpr pointer data() {
        return m_data;
    }

    [[nodiscard]] constexpr const_pointer data() const {
        return m_data;
    }

    void constexpr shrink_to_fit() {
        increase_capacity(std::max(size(), std::bit_ceil(size())));
    }

    [[nodiscard]] constexpr iterator begin() {
        return iterator(m_data);
    }
    [[nodiscard]] constexpr const_iterator cbegin() const {
        return const_iterator(m_data);
    }
    [[nodiscard]] constexpr const_iterator begin() const {
        return cbegin();
    }

    [[nodiscard]] constexpr iterator end() {
        return iterator(m_data + size());
    }
    [[nodiscard]] constexpr const_iterator cend() const {
        return const_iterator(m_data + size());
    }
    [[nodiscard]] constexpr const_iterator end() const {
        return cend();
    }

    [[nodiscard]] constexpr reverse_iterator rbegin() {
        return reverse_iterator(end());
    }
    [[nodiscard]] constexpr const_reverse_iterator crbegin() const {
        return const_reverse_iterator(cend());
    }
    [[nodiscard]] constexpr const_reverse_iterator rbegin() const {
        return crbegin();
    }

    [[nodiscard]] constexpr reverse_iterator rend() {
        return reverse_iterator(begin());
    }
    [[nodiscard]] constexpr const_reverse_iterator crend() const {
        return const_reverse_iterator(cbegin());
    }
    [[nodiscard]] constexpr const_reverse_iterator rend() const {
        return crend();
    }

    constexpr void swap(vector& other) noexcept(noexcept(std::swap(other, *this))) {
        std::swap(other, *this);
    }

    [[nodiscard]] constexpr bool operator==(const vector& other) const {
        if (size() != other.size())
            return false;
        if (other.data() == m_data)
            return true;
        auto first = begin();
        auto it = other.begin();
        auto last = end();
        while (first != last) {
            if (*first++ != *it++)
                return false;
        }
        return true;
    }

private:
    constexpr void partition(size_type index, size_type count) {
        if (count == 0)
            return;
        std::reverse(begin() + index, end());
        std::reverse(begin() + index, begin() + index + count);
        std::reverse(begin() + index + count, end());
    }

    [[nodiscard]] constexpr T* change_capacity(size_type size, size_type msize = 0) {
        T* newData = m_alloc.allocate(size);
        if constexpr (std::is_trivially_constructible_v<value_type>) {
            memcpy(newData, m_data, msize * sizeof(value_type));
        } else {
            for (size_type i = 0; i < msize; i++) {
                
                if constexpr (std::is_nothrow_move_assignable_v<value_type>) {
                    std::allocator_traits<allocator_type>::construct(m_alloc, &newData[i], std::move(m_data[i]));
                } else {
                    std::allocator_traits<allocator_type>::construct(m_alloc, &newData[i], m_data[i]);
                }
            }
        }
        return newData;
    }

    constexpr void delete_data(size_type size) {
        if constexpr (!std::is_trivially_destructible_v<value_type>) {
            for (int i = 0; i < size; i++) {
                std::allocator_traits<allocator_type>::template destroy(m_alloc, &m_data[i]);
            }
        }
    }

    constexpr void free_data(T* data) {
        std::allocator_traits<allocator_type>::deallocate(m_alloc, data, capacity());
    }

    constexpr void increase_capacity(size_type new_size) {
        T* new_data = change_capacity(std::max<size_type>(new_size, 4), size());
        delete_data(size());
        free_data(m_data);
        m_capacity = std::max<size_type>(new_size, 4);
        m_data = new_data;
    }

    T* m_data;
    size_type m_size;
    size_type m_capacity;
    [[no_unique_address]] allocator_type m_alloc;
};