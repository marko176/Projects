#pragma once
#include <iterator>
#include <type_traits>
template <typename Node>
class forward_listIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename Node::value_type;
    friend class forward_listIterator<const Node>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;


    explicit forward_listIterator(Node* data) : m_data(data) {
    }

    operator forward_listIterator<const Node>() {
        return forward_listIterator<const Node>(m_data);
    }

    [[nodiscard]] constexpr value_type& operator*() {
        return m_data->val;
    }

    [[nodiscard]] constexpr const value_type& operator*() const {
        return m_data->val;
    }

    [[nodiscard]] constexpr Node* operator->() {
        return m_data;
    }

    [[nodiscard]] constexpr const Node* operator->() const {
        return m_data;
    }

    constexpr forward_listIterator& operator++() {
        m_data = m_data->next;
        return *this;
    }

    constexpr forward_listIterator operator++(int) {
        forward_listIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    [[nodiscard]] constexpr bool operator==(const forward_listIterator& other) const noexcept {
        return m_data == other.m_data;
    }

    [[nodiscard]] constexpr bool operator!=(const forward_listIterator& other) const noexcept {
        return !(*this == other);
    }

    Node* m_data;
};