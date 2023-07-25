#pragma once
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

// 异步写日志队列

template <typename T>
class LockQueue
{
public:
    // 多个worker线程都会写日志queue
    void push(const T& data)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_queue.push(data);
        m_cond.notify_one();
    }

    // 只有一个线程读日志queue，写日志文件
    T pop()
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        while(m_queue.empty())
        {
            //  日志队列为空，线程进入wait状态
            m_cond.wait(lock);
        }

        T data = m_queue.front();
        m_queue.pop();
        return data;
    }
private:
    std::queue<T> m_queue;
    std::mutex m_mtx;
    std::condition_variable m_cond;
};