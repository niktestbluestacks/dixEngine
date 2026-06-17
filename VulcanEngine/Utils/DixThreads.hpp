#ifndef DIX_THREADS_HPP
#define DIX_THREADS_HPP

// dix
#include <Utils/DixConcepts.hpp>

// std
#include <initializer_list>
#include <iterator>
#include <mutex>
#include <thread>

// macroes

#define __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD(returnValue, name, ...)    \
    DIX_CREATE_IF_EXISTS(returnValue, name,                               \
                         Container __VA_OPT__(, ) __VA_ARGS__) {          \
        std::lock_guard<std::mutex> lock(m_mutex);                        \
        m_container.name(__VA_OPT__(DIX_RECURSIVE_TO_NAME(__VA_ARGS__))); \
    }

#define __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(returnValue, name, ...) \
    DIX_CREATE_IF_EXISTS(returnValue, name,                                   \
                         Container __VA_OPT__(, ) __VA_ARGS__) {              \
        std::lock_guard<std::mutex> lock(m_mutex);                            \
        return m_container.name(                                              \
            __VA_OPT__(DIX_RECURSIVE_TO_NAME(__VA_ARGS__)));                  \
    }

#define __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_CONST(returnValue, name, ...) \
    DIX_CREATE_IF_EXISTS_CONST(returnValue, name,                            \
                               Container __VA_OPT__(, ) __VA_ARGS__) {       \
        std::lock_guard<std::mutex> lock(m_mutex);                           \
        m_container.name(__VA_OPT__(DIX_RECURSIVE_TO_NAME(__VA_ARGS__)));    \
    }

#define __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN_CONST(returnValue, name, \
                                                            ...)               \
    DIX_CREATE_IF_EXISTS_CONST(returnValue, name,                              \
                               Container __VA_OPT__(, ) __VA_ARGS__) {         \
        std::lock_guard<std::mutex> lock(m_mutex);                             \
        return m_container.name(                                               \
            __VA_OPT__(DIX_RECURSIVE_TO_NAME(__VA_ARGS__)));                   \
    }

// macroes

namespace dix {
template <class Container>
class ThreadSafeWrapper {
   public:
    using value_type = typename Container::value_type;
    using size_type = typename Container::size_type;
    using iterator = typename Container::iterator;
    using const_iterator = typename Container::const_iterator;
    using reference = typename Container::reference;
    using const_reference = typename Container::const_reference;
    using pointer = typename Container::pointer;
    using const_pointer = typename Container::const_pointer;
    using reverse_iterator = typename Container::reverse_iterator;
    using const_reverse_iterator = typename Container::const_reverse_iterator;

    ThreadSafeWrapper() = default;
    ~ThreadSafeWrapper() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_container.~Container();
    }
    ThreadSafeWrapper(const ThreadSafeWrapper& other) {
        if (this == &other) return;
        std::lock(m_mutex, other.m_mutex);
        std::lock_guard<std::mutex> lock1(m_mutex, std::adopt_lock);
        std::lock_guard<std::mutex> lock2(other.m_mutex, std::adopt_lock);
        m_container = other.m_container;
    }
    ThreadSafeWrapper& operator=(const ThreadSafeWrapper& other) {
        if (this == &other) return *this;
        std::lock(m_mutex, other.m_mutex);
        std::lock_guard<std::mutex> lock1(m_mutex, std::adopt_lock);
        std::lock_guard<std::mutex> lock2(other.m_mutex, std::adopt_lock);
        m_container = other.m_container;
        return *this;
    }
    ThreadSafeWrapper(ThreadSafeWrapper&& other) noexcept {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_container = other.m_container;
    }
    ThreadSafeWrapper& operator=(ThreadSafeWrapper&& other) noexcept {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_container = other.m_container;
        return *this;
    }

    ThreadSafeWrapper(std::initializer_list<value_type> initList)
        : m_container{initList} {}

    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(reference, at,
                                                  ((size_type, pos)))
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN_CONST(const_reference, at,
                                                        ((size_type, pos)))
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(reference, operator[],
                                                  ((size_type, index)))
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN_CONST(
        const_reference, operator[], ((size_type, index)))
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(reference, front)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN_CONST(const_reference, front)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(reference, back)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN_CONST(const_reference, back)
    // REMOVED FOR SAFETY PURPOSES
    // __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(pointer, data)
    // __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN_CONST(const_pointer, data)
    // REMOVED FOR FASETY PURPOSES
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(iterator, begin)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN_CONST(const_iterator, begin)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN_CONST(const_iterator, cbegin)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(iterator, end)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN_CONST(const_iterator, end)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN_CONST(const_iterator, cend)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(reverse_iterator, rbegin)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN_CONST(const_reverse_iterator,
                                                        rbegin)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN_CONST(const_reverse_iterator,
                                                        rcbegin)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(reverse_iterator, rend)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN_CONST(const_reverse_iterator,
                                                        rend)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN_CONST(const_reverse_iterator,
                                                        rcend)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN_CONST(bool, empty)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN_CONST(size_type, size)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN_CONST(size_type, max_size)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD(void, reserve, ((size_type, t)))
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN_CONST(size_type, capacity)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD(void, shrink_to_fit)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD(void, clear)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(iterator, insert,
                                                  ((const_iterator, pos), (const_reference, value)))
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(iterator, insert,
                                                  ((const_iterator, pos), (value_type&&, value)))
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(iterator, insert,
                                                  ((const_iterator, pos), (size_type, count), (const_reference, value)))
    template <typename InputIt>
    constexpr iterator insert(const_iterator pos, InputIt first, InputIt last)
        requires requires (Container c, const_iterator pos, InputIt first, InputIt last) {
            c.insert(pos, first, last);
        } 
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_container.insert(pos, first, last);
    }
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(iterator, insert, 
                                                  ((const_iterator, pos), (std::initializer_list<value_type>, ilist)))

    template <typename... Args>
    constexpr iterator emplace(const_iterator pos, Args&&... args)
    requires requires(Container c, const_iterator pos, Args&&... v) {
        c.emplace(std::forward<Args>(v)...);
    }
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_container.emplace(pos, std::forward<Args>(args)...);
    }

    [[nodiscard("Erase returns valid iterator!")]]
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(iterator, erase,
                                            ((iterator, pos)))

    [[nodiscard("Erase returns valid iterator!")]]
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(iterator, erase,
                                            ((const_iterator, pos)))

    [[nodiscard("Erase returns valid iterator!")]]
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(iterator, erase,
                                            ((iterator, first), (iterator, last)))

    [[nodiscard("Erase returns valid iterator!")]]
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(iterator, erase,
                                            ((const_iterator, first), (const_iterator, last)))

    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD(void, push_back,
                                           ((const value_type&, value)))
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD(void, push_back,
                                           ((value_type&&, value)))
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD(void, push_front,
                                           ((const value_type&, value)))
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD(void, push_front,
                                           ((value_type&&, value)))

    template <typename... Args>
    constexpr decltype(auto) emplace_back(Args&&... args) 
    requires requires(Container c, Args&&... args) {
        c.emplace_back(std::forward<Args>(args)...);
    }
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_container.emplace_back(std::forward<Args>(args)...);
    }

    template <typename... Args>
    constexpr decltype(auto) emplace_front(Args&&... args)
    requires requires(Container c, Args&&... args) {
        c.emplace_front(std::forward<Args>(args)...);
    }
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_container.emplace_front(std::forward<Args>(args)...);
    }
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(void, pop_back)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(void, pop_front)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD(void, resize,
                                            ((size_type, x)))
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD(void, resize, 
                                            ((size_type, x), (const_reference, value)))
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD(void, swap, ((Container&, other)))
   private:
    mutable std::mutex m_mutex;
    Container m_container;
};
}  // namespace dix
#endif  // DIX_THREADS_HPP