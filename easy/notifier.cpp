/**
 * Created by Karol Dudzic @ 2024
 */
#include "notifier.hpp"

#include "notifierpool.hpp"


namespace easy
{

Notifier::Notifier()
    : m_uuid{getNextUuid()}
    , m_proxy{NotifiersPool::setup()}
{}

Notifier::~Notifier()
{
    NotifiersPool::teardown();
}

bool Notifier::dispatch()
{
    if (m_dispatchRecursionBarrier)
        return false;
    m_dispatchRecursionBarrier = true;

    if (auto event = m_proxy.pull(m_uuid))
    {
        auto it = m_subscriptions.find(event->uuid());
        if (it != m_subscriptions.end())
        {
            for (auto subscriber : it->second)
            {
                subscriber->notify(*event);
            }
        }
        m_dispatchRecursionBarrier = false;
        return true;
    }
    m_dispatchRecursionBarrier = false;
    return false;
}

Notifier::UUID_t Notifier::getNextUuid()
{
    static UUID_t uuid = 0;
    return ++uuid;
}

}  // namespace easy
