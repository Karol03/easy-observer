/**
 * Created by Karol Dudzic @ 2022
 */
#pragma once

#include <variant>

#include "thread.hpp"


namespace eob
{

class DispatcherThread
{
private:
    explicit DispatcherThread() = default;

public:
    template <typename T = void>
    static DispatcherThread create()
    {
        static_assert(std::is_same_v<T, void> || std::is_base_of_v<IEvent, T>);

        auto result = DispatcherThread{};
        if (!m_isThreadExists)
        {
            m_isThreadExists = true;
            if constexpr (std::is_same_v<T, void>)
            {
                result.m_thread = std::thread([&result]() { result.m_mainNotifier.run(); });
            }
            else
            {
                result.m_thread = std::thread([&result]() { result.m_mainNotifier.runUntil<T>(); });
            }
        }
        return result;
    }

    DispatcherThread(const DispatcherThread&) = delete;
    DispatcherThread& operator=(const DispatcherThread&) = delete;
    DispatcherThread(DispatcherThread&&) = default;
    DispatcherThread& operator=(DispatcherThread&&) = delete;

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
    ApplicationThread(std::function<void(Notifier&)> startupFunction)
        : m_notifier(std::move(startupFunction))
    {
        m_thread = std::thread([this]() { m_notifier.run(); });
    }

    ~ApplicationThread()
    {
        m_notifier.close();
        if (m_thread.joinable())
            m_thread.join();
    }

    template <typename T>
    inline void emit(T event)
    {
        m_notifier.emit(std::move(event));
    }

    inline operator Notifier&() noexcept
    {
        return m_notifier;
    }

private:
    Notifier m_notifier;
    std::thread m_thread;
};


class AppThread
{
public:
    AppThread(ApplicationThread* appThread)
        : m_applicationThread(std::move(appThread))
        , m_isLifetimeControl{false}
    {}

    AppThread(std::function<void(Notifier&)> appMain)
        : m_applicationThread(new ApplicationThread{std::move(appMain)})
        , m_isLifetimeControl{true}
    {}

    AppThread(const AppThread& appthread)
        : m_applicationThread(appthread.m_applicationThread)
        , m_isLifetimeControl{false}
    {}

    AppThread& operator=(const AppThread& appthread)
    {
        m_applicationThread = appthread.m_applicationThread;
        m_isLifetimeControl = false;
        return *this;
    }

    AppThread(AppThread&& appthread)
        : m_applicationThread(appthread.m_applicationThread)
        , m_isLifetimeControl{true}
    {
        appthread.m_applicationThread = nullptr;
        appthread.m_isLifetimeControl = false;
    }

    AppThread& operator=(AppThread&& appthread)
    {
        m_applicationThread = appthread.m_applicationThread;
        m_isLifetimeControl = true;
        appthread.m_applicationThread = nullptr;
        appthread.m_isLifetimeControl = false;
        return *this;
    }

    ~AppThread()
    {
        if (m_isLifetimeControl)
        {
            delete m_applicationThread;
            m_applicationThread = nullptr;
            m_isLifetimeControl = false;
        }
    }

    inline operator Notifier&() noexcept
    {
        return *(m_applicationThread);
    }

    inline operator bool() noexcept
    {
        return m_applicationThread;
    }

    inline bool isOwner() noexcept
    {
        return m_applicationThread && m_isLifetimeControl;
    }

private:
    ApplicationThread* m_applicationThread;
    bool m_isLifetimeControl;
};


template <typename T = void>
class Supervisor
{
public:
    explicit Supervisor()
        : m_dispatcher(DispatcherThread::create<T>())
    {}

    inline AppThread createAppThread(std::function<void(Notifier&)> appMain)
    {
        constexpr bool isKeepAppThreadLifetime = !std::is_same_v<T, void>;
        if constexpr (isKeepAppThreadLifetime)
        {
            static auto appThreads = std::list<AppThread>{};
            appThreads.push_back(AppThread(std::move(appMain)));
            return appThreads.back();
        }
        else
        {
            return AppThread(std::move(appMain));
        }
    }

private:
    DispatcherThread m_dispatcher;
};

}  // namespace eob
