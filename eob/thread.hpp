/**
 * Created by Karol Dudzic @ 2022
 */
#pragma once

#include <functional>
#include <thread>
#include <unordered_map>

#include "isubscription.hpp"
#include "tunnel.hpp"


namespace eob
{

class NotifyMainThread
{
public:
    static Tunnel& install(std::size_t threadId)
    {
        auto lock = std::scoped_lock(m_mutex);
        m_threads[threadId].output = &m_pipe;
        return m_threads[threadId];
    }

    static void uninstall(std::size_t threadId)
    {
        auto lock = std::scoped_lock(m_mutex);
        if (m_threads.find(threadId) != m_threads.end())
        {
            m_threads.erase(threadId);
        }
    }

    void close()
    {
        m_isClose = true;
        m_pipe.unlock();
    }

    void run()
    {
        while (!m_isClose)
        {
            m_pipe.wait();
            while (m_pipe)
            {
                auto front = m_pipe.pop();
                for (auto& [id, tunnel] : m_threads)
                {
                    tunnel.input.push(front);
                }
            }
        }
    }

    template <typename T>
    void runUntil()
    {
        auto isClose{false};
        while (!isClose)
        {
            m_pipe.wait();
            while (m_pipe)
            {
                auto front = m_pipe.pop();
                for (auto& [id, tunnel] : m_threads)
                {
                    tunnel.input.push(front);
                }
                if (Event<T>().uuid() == front->uuid())
                {
                    isClose = true;
                }
            }
        }
    }

private:
    static inline std::unordered_map<std::size_t, Tunnel> m_threads{};
    static inline std::mutex m_mutex{};
    static inline Pipe m_pipe{};
    static inline bool m_isClose{};
};


class Notifier
{
public:
    Notifier(std::function<void(Notifier&)> function = nullptr)
        : m_main{std::move(function)}
        , m_myId{generateId()}
    {
        m_tunnel = &NotifyMainThread::install(m_myId);
    }

    ~Notifier()
    {
        NotifyMainThread::uninstall(m_myId);
        m_tunnel = nullptr;
    }

    Notifier(const Notifier&) = delete;
    Notifier& operator=(const Notifier&) = delete;
    Notifier(Notifier&&) = delete;
    Notifier& operator=(Notifier&&) = delete;

    template <typename T>
    std::enable_if_t<std::is_base_of_v<IEvent, T>,
    std::function<void()>> subscribe(ISubscription* subscriber)
    {
        auto lock = std::scoped_lock(m_mutex);
        auto subID = ++m_subscriptionIds;
        m_subscriptions[subID] = subscriber;
        return [this, subID]() { unsubscribe(subID); };
    }

    template <typename T>
    std::enable_if_t<std::is_base_of_v<IEvent, T>,
    void> emit(T event)
    {
        m_tunnel->output->push(std::make_shared<T>(std::move(event)));
    }

    void close()
    {
        m_isClose = true;
        m_tunnel->input.unlock();
    }

    void run()
    {
        if (m_main)
            m_main(*this);
    }

    void wait()
    {
        while (!m_isClose && m_tunnel)
        {
            m_tunnel->input.wait();
            while (m_tunnel->input)
            {
                auto event = m_tunnel->input.pop();
                for (auto& [id, subscriber] : m_subscriptions)
                {
                    subscriber->notify(*event);
                }
            }
        }
    }

private:
    inline void unsubscribe(std::size_t id)
    {
        auto lock = std::scoped_lock(m_mutex);
        m_subscriptions.erase(id);
    }

    static inline std::size_t generateId()
    {
        static auto id = 0ul;
        return ++id;
    }

private:
    std::unordered_map<int, ISubscription*> m_subscriptions{};
    std::function<void(Notifier&)> m_main{};
    Tunnel* m_tunnel{};
    std::mutex m_mutex{};
    std::size_t m_subscriptionIds{};
    std::size_t m_myId{};
    bool m_isClose{};
};

}  // namespace eob
