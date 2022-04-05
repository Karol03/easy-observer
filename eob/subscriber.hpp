/**
 * Created by Karol Dudzic @ 2022
 */
#pragma once

#include "thread.hpp"


#include <iostream>

namespace eob
{

template <typename T>
class Subscription : public ISubscription
{
public:
    Subscription(NotifyThread* myThread)
        : m_unsubscriber{myThread->subscribe<T>(this)}
    {}

    virtual ~Subscription()
    {
        if (m_unsubscriber)
        {
            m_unsubscriber();
        }
        m_unsubscriber = nullptr;
    }

    virtual void notify(const IEvent& event)
    {
        if (Event<T>().uuid() == event.uuid())
        {
            if (auto ev = dynamic_cast<const T*>(&event))
            {
                onEvent(*ev);
            }
        }
    }

protected:
    virtual void onEvent(const T& event) = 0;

private:
    std::function<void()> m_unsubscriber;
};


template <typename... Args>
class Subscriber : public Subscription<Args>...
{
public:
    Subscriber(NotifyThread& myThread)
        : Subscription<Args>{&myThread}...
        , m_myThread{myThread}
    {}

    Subscriber(const Subscriber&) = delete;
    Subscriber& operator=(const Subscriber&) = delete;
    Subscriber(Subscriber&&) = default;
    Subscriber& operator=(Subscriber&&) = default;

protected:
    template <typename T>
    inline void emit(T event)
    {
        m_myThread.emit(std::move(event));
    }

private:
    NotifyThread& m_myThread;
};

}  // namespace eob
