/**
 * Created by Karol Dudzic @ 2022
 */
#pragma once

#include "thread.hpp"


namespace eob
{

class DispatcherThread
{
public:
    explicit DispatcherThread()
    {
        if (!m_isThreadExists)
        {
            m_isThreadExists = true;
            m_thread = std::thread([this]() { m_mainNotifier.run(); });
        }
    }

    ~DispatcherThread()
    {
        m_mainNotifier.close();
        if (m_thread.joinable())
            m_thread.join();
        m_isThreadExists = false;
    }

private:
    NotifyMainThread m_mainNotifier;
    std::thread m_thread;

    static inline std::atomic_bool m_isThreadExists{false};
};


class ApplicationThread
{
public:
    explicit ApplicationThread()
    {
        m_thread = std::thread([this]() { m_notifyThread.run(); });
    }

    ApplicationThread(std::function<void()> startupFunction)
        : m_notifyThread(std::move(startupFunction))
    {
        m_thread = std::thread([this]() { m_notifyThread.run(); });
    }

    ~ApplicationThread()
    {
        m_notifyThread.close();
        if (m_thread.joinable())
            m_thread.join();
    }

    template <typename T>
    void emit(T event)
    {
        m_notifyThread.emit(std::move(event));
    }

    operator NotifyThread&()
    {
        return m_notifyThread;
    }

private:
    NotifyThread m_notifyThread;
    std::thread m_thread;
};

}  // namespace eob
