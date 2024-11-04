#pragma once
#include "listIterator.hpp"
#include <memory>
#include <utility>

template <typename T>
struct list_node {
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    T val;
    mutable list_node* next;
    mutable list_node* prev;
};

template <typename T, class Alloc = std::allocator<T>>
class list {
public:
    template <typename U>
    friend class listIterator;
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<list_node<T>>;
    using iterator = listIterator<list_node<T>>;
    using const_iterator = listIterator<const list_node<T>>;
    using reverse_iterator = std::reverse_iterator<listIterator<list_node<T>>>;
    using const_reverse_iterator = std::reverse_iterator<listIterator<const list_node<T>>>;

    list() : m_alloc(allocator_type()), m_root(std::allocator_traits<allocator_type>::allocate(m_alloc, 1)), m_tail(std::allocator_traits<allocator_type>::allocate(m_alloc, 1)), m_size(0) {
        std::allocator_traits<allocator_type>::construct(m_alloc, m_root, value_type{}, m_tail, nullptr);
        std::allocator_traits<allocator_type>::construct(m_alloc, m_tail, value_type{}, nullptr, m_root);
    }

    explicit list(const allocator_type& alloc) : m_alloc(alloc), m_root(std::allocator_traits<allocator_type>::allocate(m_alloc, 1)), m_tail(std::allocator_traits<allocator_type>::allocate(m_alloc, 1)), m_size(0) {
        std::allocator_traits<allocator_type>::construct(m_alloc, m_root, value_type{}, m_tail, nullptr);
        std::allocator_traits<allocator_type>::construct(m_alloc, m_tail, value_type{}, nullptr, m_root);
    }

    list(size_type count, const_reference val, const allocator_type& alloc = allocator_type{}) : list(alloc) {
        assign(count, val);
    }

    template <typename It>
    list(It first, It last, const allocator_type& alloc = allocator_type{}) : list(alloc) {
        assign(first, last);
    }

    list(std::initializer_list<value_type> ilist, const allocator_type& alloc = allocator_type{}) : list(alloc) {
        assign(ilist);
    }

    ~list() {
        clear();
        if constexpr (!std::is_trivially_destructible_v<list_node<value_type>>) {
            std::allocator_traits<allocator_type>::destroy(m_alloc, m_root);
            std::allocator_traits<allocator_type>::destroy(m_alloc, m_tail);
        }
        std::allocator_traits<allocator_type>::deallocate(m_alloc, m_root, 1);
        std::allocator_traits<allocator_type>::deallocate(m_alloc, m_tail, 1);
    }

    constexpr iterator insert(iterator pos, const_reference val) {
        ++m_size;
        list_node<value_type>* temp = pos.m_data;
        list_node<value_type>* node = std::allocator_traits<allocator_type>::allocate(m_alloc, 1);
        std::allocator_traits<allocator_type>::construct(m_alloc, node, val, temp, temp->prev);
        node->prev->next = node;
        temp->prev = node;
        return iterator(node);
    }

    constexpr iterator insert(iterator pos, value_type&& val) {
        ++m_size;
        list_node<value_type>* temp = pos.m_data;
        list_node<value_type>* node = std::allocator_traits<allocator_type>::allocate(m_alloc, 1);
        std::allocator_traits<allocator_type>::construct(m_alloc, node, std::move(val), temp, temp->prev);
        node->prev->next = node;
        temp->prev = node;
        return iterator(node);
    }

    constexpr iterator insert(iterator pos, size_type count, const_reference val) {
        iterator tmp = iterator(pos.m_data->prev);
        for (size_type i = 0; i < count; i++)
            insert(pos, val);
        return ++tmp;
    }

    template <typename It>
    constexpr iterator insert(iterator pos, It first, It last) {
        iterator tmp = iterator(pos.m_data->prev);
        while (first != last)
            insert(pos, *first++);
        return ++tmp;
    }

    constexpr iterator insert(iterator pos, std::initializer_list<value_type> ilist) {
        iterator tmp = iterator(pos.m_data->prev);
        for (const_reference val : ilist)
            insert(pos, val);
        return ++tmp;
    }

    template <typename... Args>
    constexpr iterator emplace(iterator pos, Args... args) {
        ++m_size;
        list_node<value_type>* temp = pos.m_data;
        list_node<value_type>* node = std::allocator_traits<allocator_type>::allocate(m_alloc, 1);
        std::allocator_traits<allocator_type>::construct(m_alloc, node, std::forward<Args>(args)..., temp, temp->prev);
        node->prev->next = node;
        temp->prev = node;
        return iterator(node);
    }

    constexpr iterator erase(const_iterator pos) {
        --m_size;
        list_node<value_type>* toDelete = pos.m_data;
        list_node<value_type>* next = toDelete->prev->next = toDelete->next;
        toDelete->next->prev = toDelete->prev;
        if constexpr (!std::is_trivially_destructible_v<list_node<value_type>>) {
            std::allocator_traits<allocator_type>::destroy(m_alloc, toDelete);
        }
        std::allocator_traits<allocator_type>::deallocate(m_alloc, toDelete, 1);
        return next;
    }

    constexpr iterator erase(iterator first, iterator last) {
        while (first != last)
            erase(first++);
        return last;
    }

    constexpr void assign(size_type count, const_reference val) {
        // moddify the first elments the
        clear();
        for (size_type i = 0; i < count; ++i)
            push_front(val);
    }

    template <typename It>
    constexpr void assign(It first, It last) {
        // moddify the first elments the
        clear();
        while (first != last)
            push_front(*first++);
    }

    constexpr void assign(std::initializer_list<value_type> ilist) {
        // moddify the first elments the
        clear();
        for (const_reference val : ilist)
            push_front(val);
    }

    constexpr void clear() {
        while (!empty())
            pop_front();
    }

    constexpr void pop_back() {
        --m_size;
        list_node<value_type>* toDelete = m_tail->prev;
        m_tail->prev = toDelete->prev;
        m_tail->prev->next = m_tail;
        if constexpr (!std::is_trivially_destructible_v<list_node<value_type>>) {
            std::allocator_traits<allocator_type>::destroy(m_alloc, toDelete);
        }
        std::allocator_traits<allocator_type>::deallocate(m_alloc, toDelete, 1);
    }

    constexpr void pop_front() {
        --m_size;
        list_node<value_type>* toDelete = m_root->next;
        m_root->next = toDelete->next;
        m_root->next->prev = m_root;
        if constexpr (!std::is_trivially_destructible_v<list_node<value_type>>) {
            std::allocator_traits<allocator_type>::destroy(m_alloc, toDelete);
        }
        std::allocator_traits<allocator_type>::deallocate(m_alloc, toDelete, 1);
    }

    template <typename... Args>
    constexpr reference emplace_back(Args... args) {
        return *emplace(end(), std::forward<Args>(args)...);
    }

    constexpr void push_back(const_reference val) {
        insert(end(), val);
    }

    constexpr void push_back(value_type&& val) {
        insert(end(), std::move(val));
    }

    template <typename... Args>
    constexpr reference emplace_front(Args... args) {
        return *emplace(std::next(begin()), std::forward<Args>(args)...);
    }

    constexpr void push_front(const_reference val) {
        insert(std::next(begin()), val);
    }

    constexpr void push_front(value_type&& val) {
        insert(std::next(begin()), std::move(val));
    }

    void resize(size_type count, const_reference val = value_type{}) {
        while (size() < count)
            push_back(val);
        while (size() > count)
            pop_back();
    }

    [[nodiscard]] constexpr reference front() {
        return *begin();
    }

    [[nodiscard]] constexpr const_reference front() const {
        return *begin();
    }

    [[nodiscard]] constexpr reference back() {
        return *std::prev(end());
    }

    [[nodiscard]] constexpr const_reference back() const {
        return *std::prev(end());
    }

    [[nodiscard]] constexpr allocator_type get_allocator() const {
        return m_alloc;
    }

    [[nodiscard]] constexpr size_type size() const noexcept {
        return m_size;
    }

    [[nodiscard]] constexpr bool empty() const noexcept {
        return size() == 0;
    }

    [[nodiscard]] constexpr iterator begin() {
        return iterator(m_root->next);
    }

    [[nodiscard]] constexpr const_iterator cbegin() const {
        return const_iterator(m_root->next);
    }
    [[nodiscard]] constexpr const_iterator begin() const {
        return cbegin();
    }

    [[nodiscard]] constexpr iterator end() {
        return iterator(m_tail);
    }

    [[nodiscard]] constexpr const_iterator cend() const {
        return const_iterator(m_tail);
    }

    [[nodiscard]] constexpr const_iterator end() const {
        return cend();
    }

    [[nodiscard]] constexpr reverse_iterator rbegin() {
        return reverse_iterator(m_root->next);
    }

    [[nodiscard]] constexpr const_reverse_iterator crbegin() const {
        return const_reverse_iterator(m_root->next);
    }
    [[nodiscard]] constexpr const_reverse_iterator rbegin() const {
        return crbegin();
    }

    [[nodiscard]] constexpr reverse_iterator rend() {
        return reverse_iterator(m_tail);
    }

    [[nodiscard]] constexpr const_reverse_iterator crend() const {
        return const_reverse_iterator(m_tail);
    }

    [[nodiscard]] constexpr const_reverse_iterator rend() const {
        return crend();
    }

    size_type unique() {
        auto it = begin();
        size_type acc{0};
        while (std::next(it) != end()) {
            if (*it == *std::next(it)) {
                ++acc;
                erase(std::next(it));
            } else {
                ++it;
            }
        }
        return acc;
    }

    template <typename BinaryPredicate>
    size_type unique(BinaryPredicate p) {
        auto it = begin();
        size_type acc{0};
        while (std::next(it) != end()) {
            if (p(*it, *std::next(it))) {
                ++acc;
                erase(std::next(it));
            } else {
                ++it;
            }
        }
        return acc;
    }

    size_type remove(const_reference val) {
        auto it = begin();
        auto next = std::next(it);
        size_type acc{0};
        while (next != end()) {
            if (*it == val) {
                ++acc;
                erase(it);
            }
            it = next;
            ++next;
        }
        return acc;
    }

    template <typename UnaryPredicate>
    size_type remove_if(UnaryPredicate p) {
        auto it = begin();
        auto next = std::next(it);
        size_type acc{0};
        while (next != end()) {
            if (p(*it)) {
                ++acc;
                erase(it);
            }
            it = next;
            ++next;
        }
        return acc;
    }

    void splice(const_iterator pos, list& other) {
        list_node<value_type>* p = pos.m_data;
        p->prev->next = other.begin().m_data;
        p->prev->next->prev = p->prev;
        p->prev = std::prev(other.end()).m_data;
        p->prev->next = p;
        m_size += other.size();
        other.m_root->next = other.m_tail;
        other.m_tail->prev = other.m_root;
        other.m_size = 0;
    }

    void splice(const_iterator pos, list&& other) {
        list_node<value_type>* p = pos.m_data;
        p->prev->next = other.begin().m_data;
        p->prev->next->prev = p->prev;
        p->prev = std::prev(other.end()).m_data;
        p->prev->next = p;
        m_size += other.size();
        other.m_root->next = other.m_tail;
        other.m_tail->prev = other.m_root;
        other.m_size = 0;
    }

    void splice(const_iterator pos, list& other, const_iterator it) {
        list_node<value_type>* p = pos.m_data;
        it.m_data->prev->next = it.m_data->next;
        it.m_data->next->prev = it.m_data->prev;
        p->prev->next = it.m_data;
        it.m_data->prev = p->prev;
        p->prev = it.m_data;
        it.m_data->next = p;
        ++m_size;
        --other.m_size;
    }

    void splice(const_iterator pos, list&& other, const_iterator it) {
        list_node<value_type>* p = pos.m_data;
        it.m_data->prev->next = it.m_data->next;
        it.m_data->next->prev = it.m_data->prev;
        p->prev->next = it.m_data;
        it.m_data->prev = p->prev;
        p->prev = it.m_data;
        it.m_data->next = p;
        ++m_size;
        --other.m_size;
    }

    void splice(const_iterator pos, list& other, const_iterator first, const_iterator last) {
        if (other == *this)
            return;
        while (first != last) {
            splice(pos, other, first++);
            ++m_size;
            --other.m_size;
        }
    }

    void splice(const_iterator pos, list&& other, const_iterator first, const_iterator last) {
        if (other == *this)
            return;
        while (first != last) {
            splice(pos, std::move(other), first++);
            ++m_size;
            --other.m_size;
        }
    }

    void reverse() {
        m_tail = m_root;
        while (m_root->next) {
            std::swap(m_root->next, m_root->prev);
            m_root = m_root->prev;
        }
        std::swap(m_root->next, m_root->prev);
    }

    template <typename Comp = std::less<value_type>>
    void sort(Comp compare = Comp{}) {
        if (empty())
            return;

        m_root->next->prev = nullptr;
        m_tail->prev->next = nullptr;
        m_root->next = helper_merge_sort(m_root->next, m_tail->prev, compare);
        list_node<value_type>* curr = m_root;
        curr->next->prev = curr;
        while (curr->next)
            curr = curr->next;
        curr->next = m_tail;
        m_tail->prev = curr;
    }

    template <typename Compare = std::less<value_type>>
    void merge(list& other, Compare comp = Compare{}) {
        if (other.empty())
            return;
        m_root->next->prev = nullptr;
        m_tail->prev->next = nullptr;
        other.m_root->next->prev = nullptr;
        other.m_tail->prev->next = nullptr;
        m_root->next = helper_merge(m_root->next, other.m_root->next, comp);
        m_root->next->prev = m_root;
        other.m_root->next = other.m_tail;
        other.m_tail->prev = other.m_root;
        m_size += other.m_size;
        other.m_size = 0;
        list_node<value_type>* temp = m_root;
        while (temp->next) {
            temp = temp->next;
        }
        temp->next = m_tail;
        m_tail->prev = temp;
    }

    template <typename Compare = std::less<value_type>>
    void merge(list&& other, Compare comp = Compare{}) {
        if (other.empty())
            return;
        m_root->next->prev = nullptr;
        m_tail->prev->next = nullptr;
        other.m_root->next->prev = nullptr;
        other.m_tail->prev->next = nullptr;
        m_root->next = helper_merge(m_root->next, other.m_root->next, comp);
        m_root->next->prev = m_root;
        other.m_root->next = other.m_tail;
        other.m_tail->prev = other.m_root;
        m_size += other.m_size;
        other.m_size = 0;
        list_node<value_type>* temp = m_root;
        while (temp->next) {
            temp = temp->next;
        }
        temp->next = m_tail;
        m_tail->prev = temp;
    }

    [[nodiscard]] constexpr bool operator==(const list& other) const noexcept {
        if(size() != other.size())return false;
        for(auto it1 = begin(), it2 = other.begin(); it1 != end();++it1,++it2){
            if(*it1 != *it2)return false;
        }
        return true;
    }

    [[nodiscard]] constexpr bool operator!=(const list& other) const noexcept {
        return !(*this == other);
    }

private:
    template <typename Comp>
    list_node<value_type>* helper_merge_sort(list_node<value_type>* first, list_node<value_type>* last, Comp compare) {
        if (first == last)
            return first;
        list_node<value_type>* slow = first;
        list_node<value_type>* fast = first;

        while (fast != last && fast->next != last) {
            slow = slow->next;
            fast = fast->next->next;
        }

        list_node<value_type>* t = slow->next;
        slow->next = nullptr;
        t->prev = nullptr;
        list_node<value_type>* left = helper_merge_sort(first, slow, compare);
        list_node<value_type>* right = helper_merge_sort(t, last, compare);
        return helper_merge(left, right, compare);
    }

    template <typename Comp>
    list_node<value_type>* helper_merge(list_node<value_type>* left, list_node<value_type>* right, Comp compare) {
        // add compare
        if (left == nullptr)
            return right;
        if (right == nullptr)
            return left;
        list_node<value_type>* root = left;
        if (compare(left->val, right->val)) {
            left = left->next;
        } else {
            root = right;
            right = right->next;
        }
        list_node<value_type>* curr = root;
        while (left != nullptr && right != nullptr) {

            if (compare(left->val, right->val)) {
                curr->next = left;
                left->prev = curr;
                left = left->next;
            } else {
                curr->next = right;
                right->prev = curr;
                right = right->next;
            }
            curr = curr->next;
        }

        if (right != nullptr) {
            curr->next = right;
            right->prev = curr;
        } else if (left != nullptr) {
            curr->next = left;
            left->prev = curr;
        }
        return root;
    }

    [[no_unique_address]] allocator_type m_alloc;
    list_node<value_type>* m_root;
    list_node<value_type>* m_tail;
    size_type m_size;
};