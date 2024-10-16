#pragma once
#include "forward_listIterator.hpp"
#include <memory>
#include <utility>

template <typename T>
struct forward_list_node {
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    T val;
    forward_list_node* next;
};

template <typename T, class Alloc = std::allocator<T>>
class forward_list {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<forward_list_node<T>>;
    using iterator = forward_listIterator<forward_list_node<T>>;
    using const_iterator = forward_listIterator<const forward_list_node<T>>;

    forward_list() : m_alloc(allocator_type{}), m_tail(std::allocator_traits<allocator_type>::allocate(m_alloc, 1)), m_root(std::allocator_traits<allocator_type>::allocate(m_alloc, 1)) {
        std::allocator_traits<allocator_type>::construct(m_alloc, m_root, value_type{}, m_tail);
    }

    explicit forward_list(const allocator_type& alloc) : m_alloc(alloc), m_tail(std::allocator_traits<allocator_type>::allocate(m_alloc, 1)), m_root(std::allocator_traits<allocator_type>::allocate(m_alloc, 1)) {}

    explicit forward_list(size_type count, const allocator_type& alloc = allocator_type{}) : forward_list(alloc) {
        assign(count, value_type{});
    }

    template <typename It>
    forward_list(It first, It last, const allocator_type& alloc = allocator_type{}) : forward_list(alloc) {
        assign(first, last);
    }

    forward_list(std::initializer_list<value_type> ilist, const allocator_type& alloc = allocator_type{}) : forward_list(alloc) {
        assign(ilist);
    }

    // copy
    forward_list(const forward_list& other) {
        assign(other.begin(), other.end());
    }

    forward_list(forward_list&& other) noexcept : m_alloc(other.get_allocator()), m_tail(std::exchange(other.m_tail, nullptr)), m_root(std::exchange(other.m_root, nullptr)) {
    }

    forward_list& operator=(const forward_list& other) {
        forward_list temp = other;
        swap(temp);
        return *this;
    }

    forward_list& operator=(forward_list&& other) = default;

    ~forward_list() {
        clear();
        std::allocator_traits<allocator_type>::destroy(m_alloc, m_root);
        std::allocator_traits<allocator_type>::deallocate(m_alloc, m_root, 1);
        std::allocator_traits<allocator_type>::destroy(m_alloc, m_tail);
        std::allocator_traits<allocator_type>::deallocate(m_alloc, m_tail, 1);
    }

    void assign(size_type count, const_reference val) {
        // maybe not clear but change values? -> this will reduce allocations
        clear();
        while (count-- > 0)
            push_front(val);
    }

    template <typename It>
    void assign(It first, It last) {
        clear();
        while (first != last)
            push_front(*first++);
    }

    void assign(std::initializer_list<value_type> ilist) {
        clear();
        for (const_reference val : ilist)
            push_front(val);
    }

    void reverse() noexcept {
        forward_list_node<value_type>*prev = nullptr, *curr = m_root, *next = nullptr;
        m_tail = m_root;
        while (curr) {
            next = curr->next;
            curr->next = prev;
            prev = curr;
            curr = next;
        }
        m_root = prev;
    }

    template <typename Comp = std::less<value_type>>
    void sort(Comp compare = Comp{}) {
        if (empty())
            return;
        // n log n ?
        //  quick sort?
        // bottom up merge sort
        forward_list_node<T>* tmp = m_root;
        while (tmp->next != m_tail)
            tmp = tmp->next;
        tmp->next = nullptr;
        m_root->next = m_merge_sort(m_root->next, tmp, compare);
        tmp = m_root;
        while (tmp->next)
            tmp = tmp->next;
        tmp->next = m_tail;
    }

    template <class Comp = std::less<T>>
    void merge(forward_list& other, Comp compare = Comp{}) {
        if (other.empty())
            return;
        forward_list_node<T>* endR = other.end().m_data;
        m_root->next = m_helper_merge(m_root->next, other.begin().m_data, compare, m_tail, endR);
        // we didnt take
        forward_list_node<T>* curr = m_root;
        while (curr->next->next != nullptr) {
            curr = curr->next;
        }
        curr->next = m_tail;
        other.before_begin()->next = endR;
    }

    template <class Comp = std::less<T>>
    void merge(forward_list&& other, Comp compare = Comp{}) {
        if (other.empty())
            return;
        forward_list_node<T>* endR = other.end().m_data;
        m_root->next = m_helper_merge(m_root->next, other.begin().m_data, compare, m_tail, endR);
        // we didnt take
        forward_list_node<T>* curr = m_root;
        while (curr->next->next != nullptr) {
            curr = curr->next;
        }
        curr->next = m_tail;
        other.before_begin()->next = endR;
    }

    size_type remove(const_reference value) {
        if (empty())
            return 0;
        size_type acc{0};
        iterator curr = before_begin();
        iterator last = end();
        while (std::next(curr) != last) {
            if (*std::next(curr) == value) {
                erase_after(curr);
                ++acc;
            } else {
                ++curr;
            }
        }
        return acc;
    }

    template <typename UnaryPredicate>
    size_type remove_if(UnaryPredicate p) {
        if (empty())
            return 0;
        size_type acc{0};
        iterator curr = before_begin();
        iterator last = end();
        while (std::next(curr) != last) {
            if (p(*std::next(curr))) {
                erase_after(curr);
                ++acc;
            } else {
                ++curr;
            }
        }
        return acc;
    }

    size_type unique() {
        if (empty())
            return 0;
        size_type acc{0};
        iterator curr = begin();
        iterator last = end();
        while (std::next(curr) != last) {
            if (*curr == *std::next(curr)) {
                erase_after(curr);
                ++acc;
            } else {
                ++curr;
            }
        }
        return acc;
    }

    template <typename BinaryPredicate>
    size_type unique(BinaryPredicate p) {
        if (empty())
            return 0;
        size_type acc{0};
        iterator curr = begin();
        iterator last = end();
        while (std::next(curr) != last) {
            if (p(*curr, *std::next(curr))) {
                erase_after(curr);
                ++acc;
            } else {
                ++curr;
            }
        }
        return acc;
    }

    void splice_after(iterator pos, forward_list& other, iterator it) {
        iterator tmp = std::next(pos);
        if (pos == it || tmp == it)
            return;
        pos->next = it->next;
        it->next = pos->next->next;
        pos->next->next = tmp.m_data;
    }

    void splice_after(iterator pos, forward_list&& other, iterator it) {
        iterator tmp = std::next(pos);
        if (pos == it || tmp == it)
            return;
        pos->next = it->next;
        it->next = pos->next->next;
        pos->next->next = tmp.m_data;
    }

    void splice_after(iterator pos, forward_list& other) {
        splice_after(pos, other, other.before_begin(), other.end());
    }

    void splice_after(iterator pos, forward_list&& other) {
        splice_after(pos, std::move(other), other.before_begin(), other.end());
    }

    void splice_after(iterator pos, forward_list& other, iterator first, iterator last) {
        if (first == last || std::next(first) == last)
            return;
        iterator temp = std::next(pos);
        iterator next = std::next(first);
        pos->next = next.m_data;
        while (next != last) {
            pos++;
            next++;
        }
        pos->next = temp.m_data;
        first->next = last.m_data;
    }

    void splice_after(iterator pos, forward_list&& other, iterator first, iterator last) {
        if (first == last || std::next(first) == last)
            return;
        iterator temp = std::next(pos);
        iterator next = std::next(first);
        pos->next = next.m_data;
        while (next != last) {
            pos++;
            next++;
        }
        pos->next = temp.m_data;
        first->next = last.m_data;
    }

    void resize(size_type count) {
        resize(count, value_type{});
    }

    void resize(size_type count, const_reference val) {
        iterator curr = before_begin();
        while (count > 0 && curr->next != end()) {
            curr++;
            count--;
        }
        while (count > 0) {
            insert_after(curr, val);
            count--;
        }
        erase_after(curr, end());
    }

    iterator erase_after(iterator pos) {
        if (std::next(pos) == end())
            return pos;
        return erase_after(pos, std::next(std::next(pos)));
    }

    iterator erase_after(iterator first, iterator last) {

        iterator temp = first;
        iterator next = std::next(first);
        while (next != last) {
            first = next;
            next++;
            std::allocator_traits<allocator_type>::destroy(m_alloc, first.m_data);
            std::allocator_traits<allocator_type>::deallocate(m_alloc, first.m_data, 1);
        }
        temp.m_data->next = last.m_data;
        return last;
    }

    template <typename... Args>
    iterator emplace_after(iterator pos, Args&&... args) {
        forward_list_node<T>* next = std::allocator_traits<allocator_type>::allocate(m_alloc, 1);
        std::allocator_traits<allocator_type>::construct(m_alloc, next, std::forward<Args>(args)...);
        next->next = pos->next;
        pos->next = next;
        return ++pos;
    }

    iterator insert_after(iterator pos, reference val) {
        return insert_after(pos, 1, val);
    }

    iterator insert_after(iterator pos, value_type&& val) {
        forward_list_node<T>* next = std::allocator_traits<allocator_type>::allocate(m_alloc, 1);
        std::allocator_traits<allocator_type>::construct(m_alloc, next, std::move(val), pos->next);
        pos->next = next;
        return ++pos;
    }

    iterator insert_after(iterator pos, size_type count, const_reference val) {
        for (size_type i = 0; i < count; i++) {
            forward_list_node<T>* next = std::allocator_traits<allocator_type>::allocate(m_alloc, 1);
            std::allocator_traits<allocator_type>::construct(m_alloc, next, val, pos->next);
            pos->next = next;
            ++pos;
        }
        return pos;
    }

    template <typename It>
    iterator insert_after(iterator pos, It first, It last) {
        while (first != last) {
            forward_list_node<T>* next = std::allocator_traits<allocator_type>::allocate(m_alloc, 1);
            std::allocator_traits<allocator_type>::construct(m_alloc, next, *first++, pos->next);
            pos->next = next;
            ++pos;
        }
        return pos;
    }

    iterator insert_after(iterator pos, std::initializer_list<value_type> list) {
        for (const_reference val : list) {
            forward_list_node<T>* next = std::allocator_traits<allocator_type>::allocate(m_alloc, 1);
            std::allocator_traits<allocator_type>::construct(m_alloc, next, val, pos->next);
            pos->next = next;
            ++pos;
        }
        return pos;
    }

    void push_front(const_reference val) {
        forward_list_node<T>* temp = std::allocator_traits<allocator_type>::allocate(m_alloc, 1);
        std::allocator_traits<allocator_type>::construct(m_alloc, temp, val, m_root->next);
        m_root->next = temp;
    }

    void push_front(value_type&& val) {
        forward_list_node<T>* temp = std::allocator_traits<allocator_type>::allocate(m_alloc, 1);
        std::allocator_traits<allocator_type>::construct(m_alloc, temp, std::move(val), m_root->next);
        m_root->next = temp;
    }

    template <typename... Args>
    reference emplace_front(Args&&... args) {
        // change to alloc traits
        forward_list_node<T>* temp = std::allocator_traits<allocator_type>::allocate(m_alloc, 1);
        std::allocator_traits<allocator_type>::construct(m_alloc, temp, std::forward<Args>(args)..., m_root->next);
        m_root->next = temp;
        return temp->val;
    }

    void pop_front() {
        forward_list_node<T>* temp = m_root->next;
        m_root->next = temp->next;
        std::allocator_traits<allocator_type>::destroy(m_alloc, temp);
        std::allocator_traits<allocator_type>::deallocate(m_alloc, temp, 1);
    }

    [[nodiscard]] constexpr reference front() {
        return begin()->val;
    }

    [[nodiscard]] constexpr const_reference front() const {
        return begin()->val;
    }

    [[nodiscard]] constexpr allocator_type get_allocator() const {
        return m_alloc;
    }

    [[nodiscard]] constexpr bool empty() const noexcept {
        return cbegin() == cend(); // begin()  == end()
    }

    void clear() {
        while (!empty())
            pop_front();
    }

    [[nodiscard]] constexpr iterator before_begin() noexcept {
        return iterator(m_root);
    }

    [[nodiscard]] constexpr const_iterator cbefore_begin() const noexcept {
        return const_iterator(m_root);
    }

    [[nodiscard]] constexpr const_iterator before_begin() const noexcept {
        return cbefore_begin();
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

    [[nodiscard]] constexpr iterator end() noexcept {
        return iterator(m_tail);
    }

    [[nodiscard]] constexpr const_iterator cend() const noexcept {
        return const_iterator(m_tail);
    }

    [[nodiscard]] constexpr const_iterator end() const noexcept {
        return cend();
    }

    void swap(forward_list& other) noexcept {
        using std::swap;
        swap(m_root, other.m_root);
        swap(m_tail, other.m_tail);
    }

private:
    template <typename Comp>
    forward_list_node<T>* m_merge_sort(forward_list_node<T>* first, forward_list_node<T>* last, Comp compare) {
        if (first == last)
            return first;
        forward_list_node<T>* slow = first;
        forward_list_node<T>* fast = first;
        // 1 2 3 4
        while (fast != last && fast->next != last) {
            slow = slow->next;
            fast = fast->next->next;
        }
        if (fast != last)
            fast = fast->next;
        forward_list_node<T>* t = slow->next;
        slow->next = nullptr;
        forward_list_node<T>* left = m_merge_sort(first, slow, compare);
        forward_list_node<T>* right = m_merge_sort(t, fast, compare);
        return m_helper_merge(left, right, compare);
    }

    template <typename Comp>
    forward_list_node<T>* m_helper_merge(forward_list_node<T>* left, forward_list_node<T>* right, Comp compare, forward_list_node<T>* endL = nullptr, forward_list_node<T>* endR = nullptr) {
        if (left == endL)
            return right;
        if (right == endR)
            return left;
        // new list ->
        forward_list_node<T>* root = left;
        if (compare(left->val, right->val)) {
            left = left->next;
        } else {
            root = right;
            right = right->next;
        }
        root->next = nullptr;
        forward_list_node<T>* curr = root;
        while (left != endL && right != endR) {
            if (compare(left->val, right->val)) {
                curr->next = left;
                left = left->next;
            } else {
                curr->next = right;
                right = right->next;
            }
            curr = curr->next;
            curr->next = nullptr;
        }
        if (left != endL) {
            curr->next = left;
        } else {
            curr->next = right;
        }

        return root;
    }
    // using node_type = forward_list_node<T>;
    [[no_unique_address]] allocator_type m_alloc;
    forward_list_node<T>* m_tail;
    forward_list_node<T>* m_root;
};