/**
 * Created by Karol Dudzic @ 2022
 */
#pragma once

#include "isubscription.hpp"
#include "notifier.hpp"


namespace easy
{

template <typename T>
class Subscription : private ISubscription
{
public:
    Subscription(Notifier& notifier)
        : m_unsubscriber{notifier.subscribe<T>(this)}
    {}

    Subscription(const Subscription&) = delete;
    Subscription& operator=(const Subscription&) = delete;
    Subscription(Subscription&& subscription) = delete;
    Subscription& operator=(Subscription&& subscription) = delete;

    virtual ~Subscription()
    {
        if (m_unsubscriber)
        {
            m_unsubscriber();
        }
    }

protected:
    virtual void onEvent(const T& event) = 0;

private:
    void notify(const IEvent& event) override
    {
        onEvent(dynamic_cast<const T&>(event));
    }

private:
    std::function<void()> m_unsubscriber;
};


template <typename... Args>
class Subscribe : public Subscription<Args>...
{
public:
    Subscribe(Notifier& notifier)
        : Subscription<Args>{notifier}...
        , m_notifier{notifier}
    {}

    Subscribe(const Subscribe&) = delete;
    Subscribe& operator=(const Subscribe&) = delete;
    Subscribe(Subscribe&&) = delete;
    Subscribe& operator=(Subscribe&&) = delete;

protected:
    template <typename T>
    inline void publish(T event)
    {
        m_notifier.publish(std::move(event));
    }

private:
    Notifier& m_notifier;
};

}  // namespace easy
