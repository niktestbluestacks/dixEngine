#ifndef DIX_THREADS_HPP
#define DIX_THREADS_HPP

// dix
#include <Utils/DixConcepts.hpp>

// std
#include <initializer_list>
#include <iterator>
#include <mutex>
#include <thread>

#define __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD(returnValue, name, ...)    \
    DIX_CREATE_IF_EXISTS(returnValue, name,                               \
                         Container __VA_OPT__(, __VA_ARGS__)) {           \
        std::lock_guard<std::mutex> lock(m_mutex);                        \
        m_container.name(__VA_OPT__(DIX_RECURSIVE_TO_NAME(__VA_ARGS__))); \
    }

#define __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(returnValue, name, ...) \
    DIX_CREATE_IF_EXISTS(returnValue, name,                                   \
                         Container __VA_OPT__(, __VA_ARGS__)) {               \
        std::lock_guard<std::mutex> lock(m_mutex);                            \
        return m_container.name(                                              \
            __VA_OPT__(DIX_RECURSIVE_TO_NAME(__VA_ARGS__)));                  \
    }

#define __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_CONST(returnValue, name, ...) \
    DIX_CREATE_IF_EXISTS_CONST(returnValue, name,                            \
                               Container __VA_OPT__(, __VA_ARGS__)) {        \
        std::lock_guard<std::mutex> lock(m_mutex);                           \
        m_container.name(__VA_OPT__(DIX_RECURSIVE_TO_NAME(__VA_ARGS__)));    \
    }

#define __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN_CONST(returnValue, name, \
                                                            ...)               \
    DIX_CREATE_IF_EXISTS_CONST(returnValue, name,                              \
                               Container __VA_OPT__(, __VA_ARGS__)) {          \
        std::lock_guard<std::mutex> lock(m_mutex);                             \
        return m_container.name(                                               \
            __VA_OPT__(DIX_RECURSIVE_TO_NAME(__VA_ARGS__)));                   \
    }

namespace dix {
template <class Container>
class ThreadSafeWrapper {
   public:
    using value_type = typename Container::value_type;
    using size_type = typename Container::size_type;
    using iterator = typename Container::iterator;
    using const_iterator = typename Container::const_iterator;

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

    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD(void, push_back, (const value_type&, value))
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD(void, push_back, (value_type&&, value))
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD(void, push_front, (const value_type&, value))
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD(void, push_front, (value_type&&, value))
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN_CONST(bool, empty)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD(void, clear)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD(void, reserve, (size_type, t))
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD(void, resize, (size_type, x))
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(size_type, size)
    [[nodiscard("Erase returns valid iterator!")]] 
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(iterator, erase, (iterator, pos))
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(iterator, begin)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(iterator, end)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN_CONST(iterator, begin)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN_CONST(iterator, end)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(iterator, cbegin)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(iterator, bend)
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN(value_type, operator[], (size_type, index))
    __DIX_THREAD_SAFE_WRAPPER_CLASS_METHOD_RETURN_CONST(value_type, operator[], (size_type, index))

    template <typename... Args>
    void emplace_back(Args&&... args)
        requires requires(Container c, value_type v) { c.emplace_back(v); }
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_container.emplace_back(std::forward<Args>(args)...);
    }

   private:
    mutable std::mutex m_mutex;
    Container m_container;
};
}  // namespace dix
#endif  // DIX_THREADS_HPP