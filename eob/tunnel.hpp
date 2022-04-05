/**
 * Created by Karol Dudzic @ 2022
 */
#pragma once

#include <atomic>
#include <condition_variable>
#include <list>
#include <memory>
#include <mutex>

#include "event.hpp"


namespace eob
{

class Pipe
{
public:
    inline void push(std::shared_ptr<IEvent> event)
    {
        m_events.push_back(std::move(event));
        ++m_size;
        m_cv.notify_one();
    }

    inline auto pop()
    {
        --m_size;
        auto front = std::move(m_events.front());
        m_events.pop_front();
        return front;
    }

    inline void wait()
    {
        auto mutex = std::mutex{};
        auto lock = std::unique_lock{mutex};
        m_cv.wait(lock, [this]() { return m_close || m_size > 0; });
    }

    inline operator bool() const
    {
        return m_size > 0;
    }

    inline void unlock()
    {
        m_close = true;
        m_cv.notify_one();
    }

private:
    std::list<std::shared_ptr<IEvent>> m_events{};
    std::condition_variable m_cv{};
    std::atomic_uint m_size{};
    std::atomic_bool m_close{};
};


class Tunnel
{
public:
    Pipe input{};
    Pipe* output{};
};


}  // namespace eob
