#ifndef __TKS_CONCURRENCY_QUEUE_H__
#define __TKS_CONCURRENCY_QUEUE_H__

#include <condition_variable>
#include <mutex>
#include <queue>

namespace TKS::Concurrency
{
    template <typename T>
    class ConcurrentQueue
    {
    public:
        inline ConcurrentQueue(){};
        inline ~ConcurrentQueue(){};
        void push(T arg);
        T pop();
        bool empty();

    private:
        std::condition_variable m_cond;
        std::mutex m_mutex;
        std::queue<T> m_queue;
    };

    template <typename T>
    inline void ConcurrentQueue<T>::push(T arg)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        this->m_queue.push(arg);
        m_cond.notify_all();
    }

    template <typename T>
    inline T ConcurrentQueue<T>::pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cond.wait(lock, [this]()
                    { return !m_queue.empty(); });

        T item = m_queue.front();
        m_queue.pop();

        return item;
    }

    template <typename T>
    inline bool ConcurrentQueue<T>::empty()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        bool isQueueEmpty = m_queue.empty();
        m_cond.notify_all();

        return isQueueEmpty;
    }

} // namespace TKS::Concurrency

#endif //__TKS_CONCURRENCY_QUEUE_H__
