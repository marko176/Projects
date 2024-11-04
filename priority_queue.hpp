#pragma once
#include <vector>

template <typename T, class Container = std::vector<T>, class Comp = std::less<typename Container::value_type>>
class priority_queue {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using container_type = Container;
    using value_compare = Comp;
    using reference = T&;
    using const_reference = const T&;

    explicit priority_queue(const value_compare& comp, const container_type& cont) : m_compare(comp), m_cont(cont) {}

    priority_queue(const value_compare& comp) : priority_queue(comp, container_type()) {}

    priority_queue() : priority_queue(value_compare{}, container_type()) {}

    template <typename It>
    priority_queue(It first, It last, const value_compare& comp = value_compare()) : priority_queue(comp, container_type(first, last)) {
        // make heap in O(n)
        for (int i = size() / 2; i >= 0; i--) {
            heapify_down(i);
        }
    }

    template <typename... Args>
    constexpr void emplace(Args... args) {
        m_cont.emplace_back(std::forward<Args>(args)...);
        heapify_up(size() - 1);
    }

    constexpr void push(const_reference val) {
        m_cont.push_back(val);
        heapify_up(size() - 1);
    }

    constexpr void push(value_type&& val) {
        m_cont.push_back(std::move(val));
        heapify_up(size() - 1);
    }

    constexpr void pop() {
        using std::swap;
        swap(m_cont.front(), m_cont.back());
        m_cont.pop_back();
        heapify_down(0);
    }

    [[nodiscard]] constexpr const_reference top() const {
        return m_cont.front();
    }

    [[nodiscard]] constexpr size_type size() const noexcept(noexcept(m_cont.size())) {
        return m_cont.size();
    }

    [[nodiscard]] constexpr bool empty() const noexcept(noexcept(m_cont.empty())) {
        return m_cont.empty();
    }

private:
    constexpr void heapify_up(size_type index) {
        while (index) {
            size_type parent = get_parent_index(index);
            //parent < index
            if (!m_compare(m_cont[index],m_cont[parent])) {
                using std::swap;
                swap(m_cont[parent], m_cont[index]);
                index = parent;
            } else
               return;
        }
    }

    constexpr void heapify_down(size_type index) {
        while (index < size()) {
            size_type left = get_left_index(index);   // 1
            if(left >= size())return;
            size_type right = get_right_index(index); // 2
            if (right < size() && m_compare(m_cont[left],m_cont[right])) {
                left = right;
            }
            if (m_compare(m_cont[index],m_cont[left])) {
                using std::swap;
                swap(m_cont[left], m_cont[index]);
                index = left;
            } else
                return;
        }
    }

    constexpr size_type get_parent_index(size_type index) const{
        return (index - 1) / 2;
    }

    constexpr size_type get_left_index(size_type index) const{
        return index * 2 + 1;
    }
    constexpr size_type get_right_index(size_type index) const{
        return index * 2 + 2;
    }

    [[no_unique_address]] value_compare m_compare;
    container_type m_cont;
};