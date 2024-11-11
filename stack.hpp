#pragma once
#include <deque>
template <typename T, class Container = std::deque<T>>
class stack{
    public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using container_type = Container;

    [[nodiscard]] auto top() -> reference{
        return mCont.back();
    }

    [[nodiscard]] auto top() const -> const_reference{
        return mCont.back();
    }

    constexpr auto pop() -> void {
        mCont.pop_back();
    }

    template<typename... Args>
    constexpr auto emplace(Args&&... args) -> decltype(auto) {
        return mCont.emplace_back(std::forward<Args>(args)...);
    }

    constexpr auto push(const_reference val) -> void {
        mCont.push_back(val);
    }

    constexpr auto push(value_type&& val) -> void {
        mCont.push_back(std::move(val));
    }

    [[nodiscard]] constexpr auto size() const noexcept(noexcept(mCont.size())) -> size_type {
        return mCont.size();
    }

    [[nodiscard]] constexpr auto empty() const noexcept(noexcept(mCont.empty())) -> bool {
        return mCont.empty();
    }

    private:
    container_type mCont;
};