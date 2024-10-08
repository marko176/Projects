#pragma once
#include <memory>

template<typename T>
struct forward_list_node{
    T val;
    forward_list_node* next;
};

template<typename T, class Alloc = std::allocator<T>>
class forward_list{
    public:
    using value_type = T;
    using size_type = std::size_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<forward_list_node<T>>;

    forward_list() : m_alloc(allocator_type{}) , m_tail(std::allocator_traits<allocator_type>::allocate(m_alloc,1)) , m_root(m_tail) {}

    explicit forward_list(const allocator_type& alloc) : m_alloc(alloc) , m_tail(std::allocator_traits<allocator_type>::allocate(m_alloc,1)) , m_root(m_tail) {}


    [[nodiscard]] constexpr bool empty() const noexcept {
        return m_root == m_tail;//begin()  == end()
    }

    private:
    [[no_unique_address]] allocator_type m_alloc;
    forward_list_node<T>* m_tail;
    forward_list_node<T>* m_root;
};