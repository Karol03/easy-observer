#include "notifierproxy.hpp"

#include "notifierpool.hpp"


namespace easy
{

void NotifierProxy::push(UUID_t notifierUuid, std::shared_ptr<IEvent> event)
{
    NotifiersPool::push(event);

    auto notifiersSubscribedForEventUuidsIt = m_subscribedEvents.find(event->uuid());
    if (notifiersSubscribedForEventUuidsIt == m_subscribedEvents.end())
        return;

    for (const auto& subscribedNotifierUuid : notifiersSubscribedForEventUuidsIt->second)
    {
        if (subscribedNotifierUuid != notifierUuid)
        {
            m_subscribedNotifiersEventQueue[subscribedNotifierUuid].push(event);
        }
    }
}

std::shared_ptr<IEvent> NotifierProxy::pull(UUID_t notifierUuid)
{
    if (m_subscribedNotifiersEventQueue[notifierUuid].empty())
    {
        auto events = NotifiersPool::pull();
        for (const auto& event : events)
        {
            if (!m_subscribedEvents.contains(event->uuid()))    // when unsubscribed but events were in buffer
                continue;

            for (const auto& notifiersForEvent : m_subscribedEvents.at(event->uuid()))
            {
                m_subscribedNotifiersEventQueue.at(notifiersForEvent).push(event);
            }
        }
    }

    if (!m_subscribedNotifiersEventQueue[notifierUuid].empty())
    {
        auto front = m_subscribedNotifiersEventQueue[notifierUuid].front();
        m_subscribedNotifiersEventQueue[notifierUuid].pop();
        return front;
    }
    return {};
}

void NotifierProxy::subscribe(UUID_t notifierUuid, IEvent::UUID_t eventId)
{
    if (!m_subscribedEvents.contains(eventId))
    {
        NotifiersPool::subscribe(eventId);
    }
    m_subscribedEvents[eventId].insert(notifierUuid);
}

void NotifierProxy::unsubscribe(UUID_t notifierUuid, IEvent::UUID_t eventId)
{
    m_subscribedNotifiersEventQueue.erase(notifierUuid);
    m_subscribedEvents[eventId].erase(notifierUuid);

    if (m_subscribedEvents[eventId].empty())
    {
        m_subscribedEvents.erase(eventId);

        NotifiersPool::unsubscribe(eventId);
    }
}

}  // namespace
