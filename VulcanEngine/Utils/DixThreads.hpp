#ifndef DIX_THREADS_HPP
#define DIX_THREADS_HPP

// std
#include <initializer_list>
#include <iterator>
#include <mutex>
#include <thread>

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
    ThreadSafeWrapper(const ThreadSafeWrapper&) = default;
    ThreadSafeWrapper& operator=(const ThreadSafeWrapper&) = default;
    ThreadSafeWrapper(ThreadSafeWrapper&&) = default;
    ThreadSafeWrapper& operator=(ThreadSafeWrapper&&) = default;

    ThreadSafeWrapper(std::initializer_list<value_type> initList)
        : m_container{initList} {}

    void push_back(const value_type& value)
        requires requires(Container c, value_type v) { c.push_back(v); }
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_container.push_back(value);
    }

    void push_back(value_type&& value)
        requires requires(Container c, value_type v) { c.push_back(v); }
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_container.push_back(value);
    }

    void push_front(const value_type& value)
        requires requires(Container c, value_type v) { c.push_front(v); }
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_container.push_front(value);
    }

    void push_front(value_type&& value)
        requires requires(Container c, value_type v) { c.push_front(v); }
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_container.push_front(value);
    }

    template <typename... Args>
    void emplace_back(Args&&... args)
        requires requires(Container c, value_type v) { c.emplace_back(v); }
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_container.emplace_back(std::forward<Args>(args)...);
    }

    bool empty() const
        requires requires(Container c) { c.empty(); }
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_container.empty();
    }

    void clear()
        requires requires(Container c) { c.empty(); }
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_container.clear();
    }

    size_type size()
        requires requires(Container c) { c.size(); }
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_container.size();
    }

    [[nodiscard("Erase returns new, valid iterator!")]] iterator erase(iterator pos)
        requires requires(Container c, iterator pos) { c.erase(pos); }
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_container.erase(pos);
    }

    value_type operator[](size_type index) const
        requires requires(const Container c, size_type idx) { c[idx]; }
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_container[index];
    }

    value_type operator[](size_type index)
        requires requires(Container c, size_type idx) { c[idx]; }
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_container[index];
    }

    iterator begin() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_container.begin();
    }
    iterator end() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_container.end();
    }

    const_iterator begin() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_container.begin();
    }
    const_iterator end() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_container.end();
    }
    const_iterator cbegin() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_container.cbegin();
    }
    const_iterator cend() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_container.cend();
    }

   private:
    mutable std::mutex m_mutex;
    Container m_container;
};

// template <typename T>
// class ThreadSafe<std::list<T>> {
// public:
//     using value_type = T;
//     using size_type = typename std::list<T>::size_type;

//     ThreadSafe() = default;

//     void push_front(const value_type& value) {
//         std::lock_guard<std::mutex> lock(m_mutex);

//     }
// private:
//     std::mutex m_mutex;
//     std::list<T> m_container
// };
}  // namespace dix
#endif  // DIX_THREADS_HPP