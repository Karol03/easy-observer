#pragma once

#include <memory>


namespace easy
{

template <typename T>
struct DoubleEndedLinkedList
{
private:
    struct Item
    {
        friend class DoubleEndedLinkedList<T>;

    public:
        Item() : previous{nullptr}, next{nullptr} {}
        Item(const T& value) : value{value}, previous{nullptr}, next{nullptr} {}
        Item(const T& value, std::shared_ptr<Item> previous, std::shared_ptr<Item> next)
            : value{value}
            , previous{std::move(previous)}
            , next{std::move(next)}
        {}

    public:
        T value;

    private:
        std::shared_ptr<Item> previous{nullptr};
        std::shared_ptr<Item> next{nullptr};
    };

public:
    using ItemPtr = std::shared_ptr<Item>;

    class Iterator
    {
    public:
        Iterator(DoubleEndedLinkedList& list)
            : m_list{list}
            , m_value{list.m_head->next}
        {}

        Iterator(DoubleEndedLinkedList& list, ItemPtr value)
            : m_list{list}
            , m_value{value}
        {}

        T& operator*()
        {
            return m_value->value;
        }

        const T& operator*() const
        {
            return m_value->value;
        }

        T* operator->()
        {
            return &m_value->value;
        }

        const T* operator->() const
        {
            return &m_value->value;
        }

        Iterator operator++()
        {
            if (m_value->next)
            {
                auto result = Iterator(m_list, m_value->next);
                m_value = m_value->next;
                return result;
            }
            return *this;
        }

        Iterator& operator++(int)
        {
            if (m_value->next)
            {
                m_value = m_value->next;
            }
            return *this;
        }

        bool operator==(const Iterator& rhs)
        {
            return (&m_list == &rhs.m_list) && (m_value == rhs.m_value);
        }

        bool operator!=(const Iterator& rhs)
        {
            return !operator==(rhs);
        }

    private:
        DoubleEndedLinkedList& m_list;
        ItemPtr m_value;
    };

public:
    DoubleEndedLinkedList()
        : m_head{std::make_shared<Item>()}
        , m_tail{std::make_shared<Item>()}
    {
        m_head->next = m_tail;
        m_tail->previous = m_head;
    }

    DoubleEndedLinkedList(const DoubleEndedLinkedList&) = delete;
    DoubleEndedLinkedList& operator=(const DoubleEndedLinkedList&) = delete;
    DoubleEndedLinkedList(DoubleEndedLinkedList&& list)
        : m_head{list.m_head}
        , m_tail{list.m_tail}
    {
        list.m_head = nullptr;
        list.m_tail = nullptr;
    }

    DoubleEndedLinkedList& operator=(DoubleEndedLinkedList&& list)
    {
        m_head = list.m_head;
        m_tail = list.m_tail;
        list.m_head = nullptr;
        list.m_tail = nullptr;
        return *this;
    }

    inline Iterator begin()
    {
        return Iterator(*this);
    }

    inline Iterator begin() const
    {
        return Iterator(*this);
    }

    inline Iterator end()
    {
        return Iterator(*this, m_tail);
    }

    inline Iterator end() const
    {
        return Iterator(*this, m_tail);
    }

    inline ItemPtr append(const T& value)
    {
        auto item = std::make_shared<Item>(value, m_tail->previous, m_tail);
        item->previous->next = item;
        m_tail->previous = item;
        return item;
    }

    inline void remove(const ItemPtr& item)
    {
        item->next->previous = item->previous;
        item->previous->next = item->next;
    }

    inline bool empty() const
    {
        return m_head->next == m_tail;
    }

private:
    ItemPtr m_head;
    ItemPtr m_tail;
};

}  // namespace easy
