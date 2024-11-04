#pragma once
#include <iterator>
#include <type_traits>
template <typename Node>
class listIterator{
    public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = typename Node::value_type;
    friend class listIterator<const Node>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    listIterator(Node* data) : m_data(data) {}

    operator listIterator<const Node>(){
        return listIterator<const Node>(m_data);
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

    constexpr listIterator& operator++(){
        m_data=m_data->next;
        return *this;
    }

    constexpr listIterator operator++(int){
        listIterator temp = *this;
        ++(*this);
        return temp;
    }

    constexpr listIterator& operator--(){
        m_data=m_data->prev;
        return *this;
    }

    constexpr listIterator operator--(int){
        listIterator temp = *this;
        --(*this);
        return temp;
    }

    [[nodiscard]] constexpr bool operator==(const listIterator& other) const{
        return m_data == other.m_data;
    }

    [[nodiscard]] constexpr bool operator!=(const listIterator& other) const{
        return !(*this == other);
    }


    Node* m_data;
};