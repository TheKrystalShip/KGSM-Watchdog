#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <deque>

namespace TKS::Concurrency
{
    template <typename T>
    class ConcurrentQueue
    {
    public:
        ConcurrentQueue() : _queue(), _mutex(), _conditional()
        {
        }

        ~ConcurrentQueue()
        {
        }

        // Copy constructor
        ConcurrentQueue(const ConcurrentQueue &src) : _queue(src._queue)
        {
        }

        // Move constructor
        ConcurrentQueue(ConcurrentQueue &&src) : _queue(std::move(src._queue))
        {
        }

        // Copy assignment operator
        ConcurrentQueue &operator=(const ConcurrentQueue &src)
        {
            _queue = src._queue;
            return *this;
        }

        // Move assignment operator
        ConcurrentQueue &operator=(ConcurrentQueue &&src)
        {
            std::swap(_queue, src._queue);
            return *this;
        }

        /**
         *  @brief  Add data to the end of the %queue.
         *  @param  arg  Data to be added.
         *
         *  This is a typical %queue operation.  The function creates an
         *  element at the end of the %queue and assigns the given data
         *  to it.  The time complexity of the operation depends on the
         *  underlying sequence.
         */
        void push(T arg);

        /**
         *  Returns a read/write reference to the data at the first
         *  element of the %queue.
         */
        T front();

        /**
         *  @brief  Removes first element.
         *
         *  This is a typical %queue operation.  It shrinks the %queue by one.
         *  The time complexity of the operation depends on the underlying
         *  sequence.
         *
         *  Note that no data is returned, and if the first element's
         *  data is needed, it should be retrieved before pop() is
         *  called.
         */
        void pop();

        /**
         *  Returns true if the %queue is empty.
         */
        bool empty();

        /**
         * Returns the number of elements in the %queue.
         */
        std::size_t size() const;

        void swap(ConcurrentQueue<T> &q);

    private:
        std::queue<T> _queue;
        mutable std::mutex _mutex;
        std::condition_variable _conditional;
    };

    template <typename T>
    inline void ConcurrentQueue<T>::push(T arg)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        this->_queue.push(arg);
        _conditional.notify_all();
    }

    template <typename T>
    inline void ConcurrentQueue<T>::pop()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _conditional.wait(lock, [&]()
                          { return !_queue.empty(); });

        _queue.pop();
    }

    template <typename T>
    inline T ConcurrentQueue<T>::front()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _conditional.wait(lock, [&]()
                          { return !_queue.empty(); });

        return _queue.front();
    }

    template <typename T>
    inline bool ConcurrentQueue<T>::empty()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return _queue.empty();
    }

    template <typename T>
    inline std::size_t ConcurrentQueue<T>::size() const
    {
        std::unique_lock<std::mutex> lock(_mutex);
        return _queue.size();
    }

    template <typename T>
    inline void ConcurrentQueue<T>::swap(ConcurrentQueue<T> &q)
    {
        std::swap(_queue, q._queue);
    }
}
