/**
 * Created by Karol Dudzic @ 2022
 */
#pragma once

#include <memory>
#include <deque>

#include "event.hpp"


namespace easy
{

class Notifier;
class NotifierProxy;
class NotifierThreadContext;

class NotifiersPool
{
    friend class Notifier;
    friend class NotifierProxy;

private:
    static NotifierProxy& setup();
    static void teardown();

    static void push(std::shared_ptr<IEvent> event);
    static std::deque<std::shared_ptr<IEvent>> pull();

    static void subscribe(IEvent::UUID_t eventId);
    static void unsubscribe(IEvent::UUID_t eventId);

    static NotifierThreadContext& getContext();
};

}  // namespace easy
