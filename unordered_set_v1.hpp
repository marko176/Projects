#pragma once
#include <bit>
#include <cmath>
#include <list>
#include <memory>
#include <vector>
template <typename T, class Hash = std::hash<T>, class KeyEqual = std::equal_to<T>, class Allocator = std::allocator<T>>
class unordered_set_v1 {
public:
    // how allocator type works????
    using key_type = T;
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using hasher = Hash;
    using key_equal = KeyEqual;
    using allocator_type = Allocator;
    using reference = T&;
    using const_reference = const T&;
    using iterator = typename std::list<T, allocator_type>::iterator;
    using const_iterator = typename std::list<T, allocator_type>::const_iterator;
    // using node_type = std::list<T,m_alloc>::node_type;

    static constexpr size_type MIN_SIZE = sizeof(T) <= 8 ? 32 : sizeof(T) <= 16 ? 16
                                                            : sizeof(T) <= 16   ? 8
                                                                                : 4;

    unordered_set_v1() : m_list(allocator_type{}), m_vec(8, m_list.end()) {

    }

    iterator erase(iterator pos)
        requires(!std::same_as<iterator, const_iterator>)
    {
        size_type bucket = Hash{}(*pos) % m_vec.size();
        if (m_vec[bucket] == pos)
            ++m_vec[bucket];

        return m_list.erase(pos);
    }

    iterator erase(const_iterator pos) {
        size_type bucket = Hash{}(*pos) % m_vec.size();
        if (m_vec[bucket] == pos)
            ++m_vec[bucket];
        return m_list.erase(pos);
    }

    size_type erase(const_reference val) {
        iterator it = find(val);
        if (it == end())
            return 0;
        erase(it);
        return 1;
    }

    iterator erase(iterator first, iterator last) {
        iterator final = end();
        while (first != last)
            final = erase(first++);
        return final;
    }

    std::pair<iterator, iterator> equal_range(const_reference val) {
        return std::make_pair(find(val), find(val));
    }

    std::pair<const_iterator, const_iterator> equal_range(const_reference val) const {
        return std::make_pair(find(val), find(val));
    }

    size_type count(const_reference val) const {
        size_type bucket = Hash{}(val) % m_vec.size();
        iterator curr = m_vec[bucket];
        size_type acc = 0;
        while (curr != m_list.end() && Hash{}(*curr) % m_vec.size() == bucket) {
            if (*curr++ == val)
                ++acc;
        }
        return acc;
    }

    iterator find(const_reference val) {
        size_type bucket = Hash{}(val) % m_vec.size();
        iterator curr = m_vec[bucket];
        while (curr != m_list.end() && Hash{}(*curr) % m_vec.size() == bucket) {
            if (*curr++ == val)
                return --curr;
        }
        return end();
    }

    const_iterator find(const_reference val) const {
        size_type bucket = Hash{}(val) % m_vec.size();
        iterator curr = m_vec[bucket];
        while (curr != m_list.end() && Hash{}(*curr) % m_vec.size() == bucket) {
            if (*curr++ == val)
                return --curr;
        }
        return cend();
    }

    [[nodiscard]] bool contains(const_reference val) const {
        return find(val) != end();
    }

    // insert hing
    template <typename... Args>
    void emplace(Args&&... args) {
        insert(value_type(std::forward<Args>(args)...));
    }

    template <typename It>
    void insert(It first, It last) {
        while (first != last)
            insert(*first++);
    }

    void insert(std::initializer_list<value_type> ilist) {
        for (const_reference val : ilist)
            insert(val);
    }

    // when rehashing splice the nodes
    constexpr std::pair<iterator, bool> insert(const_reference val) {
        if (contains(val))
            return std::make_pair(find(val), false);
        if (size() >= Max_load_factor * m_vec.size()) {
            rehash(m_vec.size() * 2);
        }
        // rehash first??
        size_type bucket = Hash{}(val) % m_vec.size();
        return std::make_pair(m_vec[bucket] = m_list.insert(m_vec[bucket], val), true);
    }

    constexpr std::pair<iterator, bool> insert(value_type&& val) {
        if (contains(val))
            return std::make_pair(find(val), false);
        if (size() >= Max_load_factor * m_vec.size()) {
            rehash(m_vec.size() * 2);
        }
        size_type bucket = Hash{}(val) % m_vec.size();
        m_vec[bucket] = m_list.insert(m_vec[bucket], std::move(val));
    }

    void rehash(size_type count) {
        size_type size = std::bit_ceil(count);
        std::list<T, allocator_type> list;
        std::vector<typename std::list<T, allocator_type>::iterator> vec(size, m_list.end());
        auto it = m_list.begin();
        auto next = it;
        while (it != m_list.end()) {
            ++next;
            size_type bucket = Hash{}(*it) % vec.size();
            if (vec[bucket] == m_list.end())
                vec[bucket] = list.end();

            list.splice(vec[bucket], m_list, it);
            --vec[bucket];
            it = next;
        }
        m_list = std::move(list);
        m_vec = std::move(vec);
    }

    [[nodiscard]] constexpr allocator_type get_allocator() const noexcept {
        return m_alloc;
    }

    [[nodiscard]] constexpr size_type size() const noexcept {
        return m_list.size();
    }

    [[nodiscard]] constexpr bool empty() const noexcept {
        return m_list.empty();
    }

    [[nodiscard]] constexpr float load_factor() const noexcept {
        return size() / static_cast<float>(m_bucketCount);
    }

    [[nodiscard]] constexpr float max_load_factor() const noexcept {
        return Max_load_factor;
    }

    [[nodiscard]] constexpr void max_load_factor(float ml) noexcept {
        Max_load_factor = ml;
    }

    void reserve(size_type count) {
        rehash(std::ceil(count / max_load_factor()));
    }

    [[nodiscard]] hasher hash_function() const {
        return hasher{};
    }

    [[nodiscard]] key_equal key_eq() const {
        return key_equal{};
    }

    [[nodiscard]] constexpr size_type bucket_count() const noexcept {
        return m_bucketCount;
    }

    void clear() {
        m_list.clear();
        m_vec = std::vector<typename std::list<T, allocator_type>::iterator>(8, m_list.end());
    }

    [[nodiscard]] constexpr iterator begin() {
        return m_list.begin();
    }

    [[nodiscard]] constexpr const_iterator cbegin() const {
        return m_list.cbegin();
    }

    [[nodiscard]] constexpr const_iterator begin() const {
        return m_list.cbegin();
    }

    [[nodiscard]] constexpr iterator end() {
        return m_list.end();
    }

    [[nodiscard]] constexpr const_iterator cend() const {
        return m_list.cend();
    }

    [[nodiscard]] constexpr const_iterator end() const {
        return m_list.cend();
    }

private:
    float Max_load_factor = 0.8f;
    size_type m_bucketCount;
    allocator_type m_alloc;
    std::list<T, allocator_type> m_list;
    std::vector<typename std::list<T, allocator_type>::iterator> m_vec;
};