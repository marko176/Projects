#pragma once
#include <memory>
#include <algorithm>
#include <bit>
template<typename T,class Alloc = std::allocator<T>>
class vector{
    public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using allocator_type = Alloc;

    vector() : m_data(nullptr), m_size(0), m_capacity(0), m_alloc{} {}

    vector(size_type size) : m_data(change_capacity(std::bit_ceil(size))) , m_size(size) , m_capacity(std::bit_ceil(size)) , m_alloc{} {}
    
    ~vector() noexcept{
        delete_data(m_size);
        free_data(m_data);
    }

    void constexpr push_back(const_reference val){
        if(size()>=capacity())
            increase_capacity(capacity()*2);
        
        new(&m_data[m_size++]) value_type(val);
    }

    void constexpr push_back(value_type&& val){
        if(size()>=capacity())
            increase_capacity(capacity()*2);
        
        new(&m_data[m_size++]) value_type(std::move(val));
    }

    template <typename... Args>
    reference constexpr emplace_back(Args&&... args){
        if(size()>=capacity())
            increase_capacity(capacity()*2);
        
        return new(&m_data[m_size++]) value_type(std::forward<Args>(args)...);
    }

    void constexpr clear() noexcept(noexcept(~value_type())){
        delete_data(size());
        m_size = 0;
    }

    void constexpr pop_back(){
        m_data[--m_size].~value_type();
    }

    void reserve(size_type size){
        increase_capacity(std::bit_ceil(size));
    }

    [[nodiscard]] constexpr reference operator[](size_type index){
        return m_data[index];
    }    
    [[nodiscard]] constexpr const_reference operator[](size_type index) const{
        return m_data[index];
    }

    [[nodiscard]] constexpr reference at(size_type index){
        if(index>=m_size)throw std::out_of_range{};
        return m_data[index];
    }    
    [[nodiscard]] constexpr const_reference at(size_type index) const{
        if(index>=m_size)throw std::out_of_range{};
        return m_data[index];
    }

    [[nodiscard]] constexpr reference front(size_type index){
        return m_data[0];
    }    
    [[nodiscard]] constexpr const_reference front(size_type index) const{
        return m_data[0];
    }

    [[nodiscard]] constexpr reference back(size_type index){
        return m_data[size()-1];
    }    
    [[nodiscard]] constexpr const_reference back(size_type index) const{
        return m_data[size()-1];
    }

    [[nodiscard]] constexpr size_type size() const noexcept{
        return m_size;
    }
 
    [[nodiscard]] constexpr size_type max_size() const noexcept{
        return UINT64_MAX;//?
    }

    [[nodiscard]] constexpr size_type capacity() const noexcept{
        return m_capacity;
    }

    [[nodiscard]] constexpr bool empty() const noexcept{
        return size()==0;
    }   

    [[nodiscard]] constexpr pointer data() {
        return m_data;
    }

    [[nodiscard]] constexpr const_pointer data() const{
        return m_data;
    }

    void constexpr shrink_to_fit(){
        increase_capacity(std::max(size(),std::bit_ceil(size())));
    }

    private:

    [[nodiscard]] T* change_capacity(size_type size, size_type msize = 0){
        T* newData = (T*)::operator new(size * sizeof(value_type));
        for(int i = 0;i<msize;i++){
            if constexpr(std::is_trivially_constructible_v<value_type>){
                std::memcpy(newData,m_data,msize*sizeof(value_type));
            }else if constexpr(std::is_nothrow_move_assignable_v<value_type>){
                new(&newData[i]) value_type(std::move(m_data[i]));
            }else{
                new(&newData[i]) value_type(m_data[i]);//move or memcpy
            }
        }
        return newData;
    }

    void delete_data(size_type size){
        if constexpr(!std::is_trivially_destructible_v<value_type>){
            for(int i = 0;i<size;i++){
                m_data[i].~value_type();
            }
        }
    }

    void free_data(T* data){
        ::operator delete[](data);
    }

    void constexpr increase_capacity(size_type new_size) {
        T* new_data = change_capacity(std::max<size_type>(new_size,4),size());
        m_capacity = std::max<size_type>(new_size,4);
        delete_data(size());
        m_data = new_data;
    }

    T* m_data;
    size_type m_size;
    size_type m_capacity;
    [[no_unique_address]] allocator_type m_alloc;

};