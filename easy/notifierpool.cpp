#include "notifierpool.hpp"

#include <thread>
#include <mutex>
#include <shared_mutex>

#include "notifier.hpp"
#include "notifierproxy.hpp"
#include "notifierthreadcontext.hpp"


namespace easy
{

inline static std::map<std::thread::id, NotifierThreadContext> setupNotifiers = {};
inline static std::shared_mutex poolAccessMutex = {};


NotifierProxy& NotifiersPool::setup()
{
    const auto id = std::this_thread::get_id();
    std::unique_lock lockWrite(poolAccessMutex);
    if (!setupNotifiers.contains(id))
    {
        setupNotifiers.try_emplace(id, NotifierThreadContext());
    }
    auto& context = setupNotifiers.at(id);
    ++context.referenceCounter;
    return context.proxy;
}

void NotifiersPool::teardown()
{
    const auto id = std::this_thread::get_id();
    std::unique_lock lockWrite(poolAccessMutex);
    auto& context = setupNotifiers.at(id);
    if (--context.referenceCounter == 0)
    {
        setupNotifiers.erase(id);
    }
}

void NotifiersPool::push(std::shared_ptr<IEvent> event)
{
    std::unique_lock lockWrite(poolAccessMutex);
    for (auto& [id, notifier] : setupNotifiers)
    {
        if (id == std::this_thread::get_id())
            continue;

        if (notifier.subscribedEvents.contains(event->uuid()))
        {
            notifier.events.push_back(event);
        }
    }
}

std::deque<std::shared_ptr<IEvent>> NotifiersPool::pull()
{
    auto events = std::deque<std::shared_ptr<IEvent>>{};
    std::shared_lock lockRead(poolAccessMutex);
    auto& context = setupNotifiers.at(std::this_thread::get_id());
    std::swap(events, context.events);
    return events;
}

void NotifiersPool::subscribe(IEvent::UUID_t eventId)
{
    std::shared_lock lockRead(poolAccessMutex);
    auto& context = setupNotifiers.at(std::this_thread::get_id());
    context.subscribedEvents.insert(eventId);
}

void NotifiersPool::unsubscribe(IEvent::UUID_t eventId)
{
    std::shared_lock lockRead(poolAccessMutex);
    auto& context = setupNotifiers.at(std::this_thread::get_id());
    context.subscribedEvents.erase(eventId);
}

}  // namespace
