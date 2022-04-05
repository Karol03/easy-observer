/**
 * Created by Karol Dudzic @ 2022
 */
#pragma once

#include <utility>


namespace eob
{

struct IEvent
{
    virtual ~IEvent() = default;
    virtual std::size_t uuid() const = 0;

protected:
    inline static std::size_t generateUuid()
    {
        static auto uuid = std::size_t{};
        return ++uuid;
    }
};

template <typename T>
struct Event : public IEvent
{
    Event() { if (!m_uuid) m_uuid = generateUuid(); }
    virtual std::size_t uuid() const { return m_uuid; }

private:
    static inline std::size_t m_uuid{};
};

}  // namespace eob
