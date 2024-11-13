/**
 * Created by Karol Dudzic @ 2022
 */
#pragma once

#include <inttypes.h>


namespace easy
{

struct IEvent
{
    using UUID_t = uint64_t;

    virtual ~IEvent() = default;

    virtual UUID_t uuid() const = 0;

protected:
    inline static UUID_t generateUuid()
    {
        static auto uuid = UUID_t{};
        return ++uuid;
    }
};

template <typename T>
struct Event : public IEvent
{
    Event() { init(); }
    static UUID_t UUID() { init(); return m_uuid; }
    virtual UUID_t uuid() const override { return m_uuid; }

private:
    inline static void init() { if (!m_uuid) m_uuid = generateUuid(); }

    static inline UUID_t m_uuid = {};
};

}  // namespace easy
