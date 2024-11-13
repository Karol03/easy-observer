/**
 * Created by Karol Dudzic @ 2024
 */
#pragma once

#include <deque>
#include <set>

#include "notifierproxy.hpp"


namespace easy
{

class NotifierThreadContext
{
public:
    unsigned referenceCounter{0};
    NotifierProxy proxy{};
    std::deque<std::shared_ptr<IEvent>> events;
    std::set<IEvent::UUID_t> subscribedEvents;
};

}  // namespace easy
