/**
 * Created by Karol Dudzic @ 2022
 */
#pragma once

#include <memory>
#include <queue>
#include <set>
#include <map>

#include "event.hpp"


namespace easy
{

class NotifierProxy
{
    friend class NotifierThreadContext;

public:
    using UUID_t = uint64_t;

private:
    NotifierProxy() = default;

public:
    void push(UUID_t notifierUuid, std::shared_ptr<IEvent> event);
    std::shared_ptr<IEvent> pull(UUID_t notifierUuid);

    void subscribe(UUID_t notifierUuid, IEvent::UUID_t eventId);
    void unsubscribe(UUID_t notifierUuid, IEvent::UUID_t eventId);

private:
    std::map<IEvent::UUID_t, std::set<UUID_t>> m_subscribedEvents;
    std::map<UUID_t, std::queue<std::shared_ptr<IEvent>>> m_subscribedNotifiersEventQueue;
};

}  // namespace easy
