#pragma once
#include "dequeIterator.hpp"
#include <bit>
#include <concepts>
#include <cstring>
#include <iterator>
#include <memory>
#include <utility>
template <typename T, class Alloc = std::allocator<T>>
class deque {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using allocator_type = Alloc;
    using array_allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<T*>;
    using iterator = dequeIterator<T>;
    using const_iterator = dequeIterator<const T>;
    using reverse_iterator = std::reverse_iterator<dequeIterator<T>>;
    using const_reverse_iterator = std::reverse_iterator<dequeIterator<const T>>;
    static constexpr size_type block_size = dequeIterator<value_type>::block_size;
    // using block size from iterator

    deque() : m_alloc{}, m_before_first{block_size / 2}, m_after_last{block_size / 2 + 1}, m_size{0}, m_data{nullptr} { // 4 , 5
        grow_right(2);
    }

    explicit deque(allocator_type alloc) : m_alloc{alloc}, m_before_first{block_size / 2}, m_after_last{block_size / 2 + 1}, m_size{0}, m_data{nullptr} { // 4 , 5
        grow_right(2);
    }

    explicit deque(size_type count, allocator_type alloc = allocator_type{}) : m_alloc{alloc}, m_before_first{block_size / 2}, m_after_last{block_size / 2 + 1}, m_size{0}, m_data{nullptr} { // 4 , 5
        grow_right(std::max<size_type>(count / block_size + 1, 2));
        assign(count, value_type{});
    }

    deque(size_type count, const_reference val, allocator_type alloc = allocator_type{}) : m_alloc{alloc}, m_before_first{block_size / 2}, m_after_last{block_size / 2 + 1}, m_size{0}, m_data{nullptr} { // 4 , 5
        grow_right(std::max<size_type>(count / block_size + 1, 2));
        assign(count, val);
    }

    template <typename It>
    deque(It first, It last, allocator_type alloc = allocator_type{})
        requires requires(It it) { It::iterator_category; }
        : m_alloc{alloc}, m_before_first{block_size / 2}, m_after_last{block_size / 2 + 1}, m_size{0}, m_data{nullptr} { // 4 , 5
        if constexpr (std::is_same_v<typename It::iterator_category, std::random_access_iterator_tag>) {
            grow_right(std::max<size_type>((last - first) / block_size + 1, 2));
        } else {
            grow_right(2);
        }
        assign(first, last);
    }

    deque(const deque& other) : deque(other.cbegin(), other.cend()) {
    }

    deque(deque&& other) noexcept : m_alloc{other.m_alloc}, m_before_first{std::exchange(other.m_before_first, block_size / 2)}, m_after_last{std::exchange(other.m_after_last, block_size / 2 + 1)}, m_size{std::exchange(other.m_size, 0)}, m_data{std::exchange(other.m_data, nullptr)} {
    }

    deque& operator=(deque other) {
        swap(other);
        return *this;
    }

    deque(std::initializer_list<value_type>& ilist, const allocator_type& alloc = allocator_type{}) : m_alloc{alloc}, m_before_first{block_size / 2}, m_after_last{block_size / 2 + 1}, m_size{0}, m_data{nullptr} { // 4 , 5
        grow_right(std::max<size_type>(ilist.size() / block_size + 1, 2));
        assign(ilist);
    }

    ~deque() {
        clear();
        deallocate(0, m_size);
        array_allocator{}.deallocate(m_data, m_size);
    }

    auto shrink_to_fit() -> void {
        // WARNING NEED TO FIX ! !
        difference_type first = m_before_first / block_size; // block with before first element
        difference_type last = m_after_last / block_size;    // block with after last element
        size_type count = last - first + 1;
        size_type new_size = std::bit_ceil(count + 1);
        if (new_size == m_size)
            return;
        T** new_data = array_allocator{}.allocate(new_size);
        std::memcpy(new_data + (new_size - count), m_data + first, count * sizeof(T*)); // what if it is bigger
        size_type i = 0;
        size_type j = 0;
        for (; i < m_size && j < new_size - count; i++) {
            if (i < first || i > last) {
                new_data[j++] = m_data[i];
            }
        }
        for (; j < new_size - count; j++) {
            new_data[j] = std::allocator_traits<allocator_type>::allocate(m_alloc, block_size);
        }

        for (; i < m_size; i++) {
            if (i < first || i > last) {
                std::allocator_traits<allocator_type>::deallocate(m_alloc, m_data[i], block_size);
            }
        }
        // first is curr positiion
        // new_size - count is next position
        m_before_first += block_size * (new_size - count - first);
        m_after_last += block_size * (new_size - count - first);
        array_allocator{}.deallocate(m_data, m_size);
        m_data = new_data;
        m_size = new_size;
        // need to free memory
    }

    iterator erase(const_iterator pos) {
        return erase(pos, std::next(pos));
    }

    iterator erase(const_iterator first, const_iterator last) {
        for (int i = first - cbegin(), j = last - cbegin(); j < size(); i++, j++) {
            this->operator[](i) = std::move(this->operator[](j));
        }
        m_size -= (last - first);
        return iterator(m_data, first - cbegin());
    }

    void assign(size_type count, const_reference val) {
        clear();
        while (count--)
            push_back(val);
    }

    template <typename It>
    void assign(It first, It last) {
        clear();
        while (first != last)
            push_back(*first++);
    }

    void assign(std::initializer_list<value_type>& ilist) {
        clear();
        for (const_reference val : ilist)
            push_back(val);
    }

    void resize(size_type count, const_reference val = value_type{}) {
        count = std::bit_ceil(count);
        while (size() > count)
            pop_back();
        while (size() < count)
            push_back(val);
    }

    iterator insert(const_iterator pos, const_reference val) {
        return insert(pos, 1, val);
    }

    iterator insert(const_iterator pos, size_type count, const_reference val) {
        // work needen
        // check if const_iterator works becouse we need just index not iterator
        using std::swap;
        size_type index = pos - cbegin();
        for (size_type i = 0; i < count; i++)
            push_back(val);
        size_type end = size() - 1;
        for (size_type i = index; i < end; i++, end--)
            swap(this->operator[](i), this->operator[](end));
        end = index + count - 1;
        for (size_type i = index; i < end; i++, end--)
            swap(this->operator[](i), this->operator[](end));
        end = size() - 1;
        for (size_type i = index + count; i < end; i++, end--)
            swap(this->operator[](i), this->operator[](end));
        return begin() + index;
    }

    iterator insert(const_iterator pos, value_type&& val) {
        // work needen
        // check if const_iterator works becouse we need just index not iterator
        using std::swap;
        size_type index = pos - cbegin();
        push_back(std::move(val));
        size_type end = size() - 1;

        for (size_type i = index; i < end; i++, end--)
            swap(this->operator[](i), this->operator[](end));
        end = size() - 1;
        for (size_type i = index + 1; i < end; i++, end--)
            swap(this->operator[](i), this->operator[](end));
        return begin() + index;
    }

    template <typename It>
    iterator insert(const_iterator pos, It first, It last)
        requires requires(It) { It::iterator_category; }
    {
        // requires iterator trait
        //  work needen
        //  check if const_iterator works becouse we need just index not iterator
        using std::swap;
        size_type index = pos - cbegin();
        size_type count = 0;
        while (first != last)
            push_back(*first++);
        size_type end = size() - 1;
        for (size_type i = index; i < end; i++, end--)
            swap(this->operator[](i), this->operator[](end));
        end = index + count - 1;
        for (size_type i = index; i < end; i++, end--)
            swap(this->operator[](i), this->operator[](end));
        end = size() - 1;
        for (size_type i = index + count; i < end; i++, end--)
            swap(this->operator[](i), this->operator[](end));
        return begin() + index;
    }

    iterator insert(const_iterator pos, std::initializer_list<value_type>& ilist) {
        // work needen
        // check if const_iterator works becouse we need just index not iterator
        using std::swap;
        size_type index = pos - cbegin();
        size_type count = 0;

        for (const_reference val : ilist)
            push_back(val);
        size_type end = size() - 1;
        for (size_type i = index; i < end; i++, end--)
            swap(this->operator[](i), this->operator[](end));
        end = index + count - 1;
        for (size_type i = index; i < end; i++, end--)
            swap(this->operator[](i), this->operator[](end));
        end = size() - 1;
        for (size_type i = index + count; i < end; i++, end--)
            swap(this->operator[](i), this->operator[](end));
        return begin() + index;
    }

    template <typename... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        // work needen
        // check if const_iterator works becouse we need just index not iterator
        using std::swap;
        size_type index = pos - cbegin();
        emplace_back(std::forward<Args>(args)...);
        size_type end = size() - 1;
        for (size_type i = index; i < end; i++, end--)
            swap(this->operator[](i), this->operator[](end));
        end = size() - 1;
        for (size_type i = index + 1; i < end; i++, end--)
            swap(this->operator[](i), this->operator[](end));
        return begin() + index;
    }

    [[nodiscard]] reference front() {
        return this->operator[](0);
    }

    [[nodiscard]] const_reference front() const {
        return this->operator[](0);
    }

    [[nodiscard]] reference back() {
        return this->operator[](size() - 1);
    }

    [[nodiscard]] const_reference back() const {
        return this->operator[](size() - 1);
    }

    [[nodiscard]] reference at(size_type pos) {
        if (pos >= size())
            throw std::out_of_range{"Deque index out of range!"};
        return this->operator[](pos);
    }

    [[nodiscard]] const_reference at(size_type pos) const {
        if (pos >= size())
            throw std::out_of_range{"Deque index out of range!"};
        return this->operator[](pos);
    }

    [[nodiscard]] constexpr reference operator[](size_type pos) {
        pos += m_before_first + 1;
        return m_data[pos / block_size][pos % block_size];
    }

    [[nodiscard]] constexpr const_reference operator[](size_type pos) const {
        pos += m_before_first + 1;
        return m_data[pos / block_size][pos % block_size];
    }

    template <typename... Args>
    reference emplace_back(Args&&... args) {
        std::allocator_traits<allocator_type>::construct(m_alloc, &(this->operator[](size())), std::forward<Args>(args)...);
        if (++m_after_last >= block_size * m_size)
            grow_right(m_size * 2);
    }

    constexpr void push_back(const_reference val) {
        std::allocator_traits<allocator_type>::construct(m_alloc, &(this->operator[](size())), val);
        if (++m_after_last >= block_size * m_size)
            grow_right(m_size * 2);
    }

    constexpr void push_back(value_type&& val) {
        std::allocator_traits<allocator_type>::construct(m_alloc, &(this->operator[](size())), std::move(val));
        if (++m_after_last >= block_size * m_size)
            grow_right(m_size * 2);
    }

    template <typename... Args>
    reference emplace_front(Args&&... args) {
        std::allocator_traits<allocator_type>::construct(m_alloc, &(m_data[m_before_first / block_size][m_before_first % block_size]), std::forward<Args>(args)...);
        if (--m_before_first <= 1)
            grow_left(m_size * 2);
    }

    constexpr void push_front(const_reference val) {
        std::allocator_traits<allocator_type>::construct(m_alloc, &(m_data[m_before_first / block_size][m_before_first % block_size]), val);
        if (--m_before_first <= 1)
            grow_left(m_size * 2);
    }

    constexpr void push_front(value_type&& val) {
        std::allocator_traits<allocator_type>::construct(m_alloc, &(m_data[m_before_first / block_size][m_before_first % block_size]), std::move(val));
        if (--m_before_first <= 1)
            grow_left(m_size * 2);
    }

    auto pop_back() -> void {
        --m_after_last;
        if constexpr (!std::is_trivially_destructible_v<value_type>) {
            std::allocator_traits<allocator_type>::destroy(m_alloc, &(this->operator[](size())));
        }
    }

    auto pop_front() -> void {
        ++m_before_first;
        if constexpr (!std::is_trivially_destructible_v<value_type>) {
            std::allocator_traits<allocator_type>::destroy(m_alloc, &(m_data[m_before_first / block_size][m_before_first % block_size]));
        }
    }

    // array of ptr to blocks
    void grow_right(size_type new_size) {
        new_size = std::bit_ceil(new_size);
        T** new_data = array_allocator{}.allocate(new_size);
        std::memcpy(new_data, m_data, m_size * sizeof(T*));
        for (size_type i = m_size; i < new_size; i++) {
            new_data[i] = m_alloc.allocate(block_size);
        }
        array_allocator{}.deallocate(m_data, m_size);
        m_data = new_data;
        m_size = new_size;
    }

    void grow_left(size_type new_size) {
        new_size = std::bit_ceil(new_size);
        size_type offset = new_size - m_size;
        T** new_data = array_allocator{}.allocate(new_size);
        std::memcpy(new_data + offset, m_data, m_size * sizeof(T*));
        for (size_type i = 0; i < offset; i++) {
            new_data[i] = m_alloc.allocate(block_size);
        }
        array_allocator{}.deallocate(m_data, m_size);
        m_data = new_data;
        m_size = new_size;
        m_after_last += offset * block_size;
        m_before_first += offset * block_size;
    }

    void deallocate(int first, int last) {
        while (first != last) {
            std::allocator_traits<allocator_type>::deallocate(m_alloc, m_data[first++], block_size);
        }
    }

    // destroy is pop_back

    [[nodiscard]] constexpr allocator_type get_allocator() const noexcept {
        return m_alloc;
    }

    [[nodiscard]] constexpr size_type size() const noexcept {
        return m_after_last - m_before_first - 1;
    }

    [[nodiscard]] constexpr auto empty() const noexcept -> bool {
        return size() == 0;
    }

    void clear() {
        while (!empty())
            pop_back();
    }

    [[nodiscard]] constexpr iterator begin() {
        return iterator(m_data, m_before_first + 1);
    }

    [[nodiscard]] constexpr const_iterator cbegin() const {
        return const_iterator(const_cast<const T**>(m_data), m_before_first + 1);
    }

    [[nodiscard]] constexpr iterator begin() const {
        return cbegin();
    }

    [[nodiscard]] constexpr reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    [[nodiscard]] constexpr const_reverse_iterator crbegin() const {
        return const_reverse_iterator(cend());
    }

    [[nodiscard]] constexpr reverse_iterator rbegin() const {
        return crbegin();
    }

    [[nodiscard]] constexpr iterator end() {
        return iterator(m_data, m_after_last);
    }

    [[nodiscard]] constexpr const_iterator cend() const {
        return const_iterator(const_cast<const T**>(m_data), m_after_last);
    }

    [[nodiscard]] constexpr iterator end() const {
        return cend();
    }

    [[nodiscard]] constexpr reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    [[nodiscard]] constexpr const_reverse_iterator crend() const {
        return const_reverse_iterator(cbegin());
    }

    [[nodiscard]] constexpr reverse_iterator rend() const {
        return crend();
    }

    constexpr auto swap(deque& other) noexcept -> void {
        using std::swap;
        swap(other.m_alloc, m_alloc);
        swap(other.m_before_first, m_before_first);
        swap(other.m_after_last, m_after_last);
        swap(other.m_size, m_size);
        swap(other.m_data, m_data);
    }

private:
    [[no_unique_address]] allocator_type m_alloc;
    size_type m_before_first;
    size_type m_after_last;
    size_type m_size;
    T** m_data;
};