/**
 * Created by Karol Dudzic @ 2022
 */
#pragma once

#include "event.hpp"


namespace eob
{

class ISubscription
{
public:
    virtual ~ISubscription() = default;
    virtual void notify(const IEvent& event) = 0;
};

}  // namespace eob
