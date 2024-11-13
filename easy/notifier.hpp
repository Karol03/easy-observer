/**
 * Created by Karol Dudzic @ 2022
 */
#pragma once

#include <functional>
#include <map>

#include "notifierproxy.hpp"
#include "isubscription.hpp"
#include "doubleendedlinkedlist.hpp"


namespace easy
{

class Notifier
{
    friend class ISubscription;

    using SubscriptionsList = DoubleEndedLinkedList<ISubscription*>;
    using UUID_t = uint64_t;

public:
    Notifier();
    ~Notifier();
    Notifier(const Notifier&) = delete;
    Notifier& operator=(const Notifier&) = delete;
    Notifier(Notifier&&) = delete;
    Notifier& operator=(Notifier&&) = delete;

    template <typename T>
    std::enable_if_t<std::is_base_of_v<IEvent, T>,
    std::function<void()>> subscribe(ISubscription* subscriber)
    {
        const auto eventId = T::UUID();
        if (!m_subscriptions.contains(eventId))
        {
            m_proxy.subscribe(m_uuid, eventId);
        }
        auto item = m_subscriptions[eventId].append(subscriber);
        return [item = std::move(item), eventId, this]() {
            m_subscriptions[eventId].remove(item);
            if (m_subscriptions[eventId].empty())
            {
                m_proxy.unsubscribe(m_uuid, eventId);
                m_subscriptions.erase(eventId);
            }
        };
    }

    template <typename T>
    std::enable_if_t<std::is_base_of_v<IEvent, T>,
    void> publish(T event)
    {
        m_proxy.push(m_uuid, std::make_shared<T>(std::move(event)));
    }

    bool dispatch();

private:
    static UUID_t getNextUuid();

private:
    std::map<IEvent::UUID_t, SubscriptionsList> m_subscriptions;
    UUID_t m_uuid;
    NotifierProxy& m_proxy;
    bool m_dispatchRecursionBarrier = false;
};

}  // namespace easy
