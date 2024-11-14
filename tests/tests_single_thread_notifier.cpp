#include "catch2/catch_amalgamated.hpp"
#include "easy/subscriber.hpp"
#include "easy/notifier.hpp"

#include <thread>


namespace single_thread
{

class EventThread : public easy::Event<EventThread>
{
public:
    EventThread()
        : threadId{std::this_thread::get_id()}
    {}

    bool isCalledInSenderThread() const { return std::this_thread::get_id() == threadId; }

public:
    std::thread::id threadId;
};

class EventThread2 : public easy::Event<EventThread2>
{
public:
    EventThread2()
        : threadId{std::this_thread::get_id()}
    {}

    bool isCalledInSenderThread() const { return std::this_thread::get_id() == threadId; }

public:
    std::thread::id threadId;
};

class EventThread3 : public easy::Event<EventThread3>
{
public:
    EventThread3()
        : threadId{std::this_thread::get_id()}
    {}

    bool isCalledInSenderThread() const { return std::this_thread::get_id() == threadId; }

public:
    std::thread::id threadId;
};

class EventThread4 : public easy::Event<EventThread4>
{
public:
    EventThread4()
        : threadId{std::this_thread::get_id()}
    {}

    bool isCalledInSenderThread() const { return std::this_thread::get_id() == threadId; }

public:
    std::thread::id threadId;
};

template <typename T>
class NeverCalledSubscriber : public easy::Subscribe<T>
{
public:
    NeverCalledSubscriber(easy::Notifier& notifier) : easy::Subscribe<T>{notifier} {}
    ~NeverCalledSubscriber() { REQUIRE_FALSE(isCalled); }
    void onEvent(const T& event) { REQUIRE_FALSE(event.isCalledInSenderThread()); isCalled = true; }

private:
    bool isCalled = false;
};

class SubscriberWithDispatchOnEvent : public easy::Subscribe<EventThread>
{
public:
    SubscriberWithDispatchOnEvent(easy::Notifier& notifier)
        : easy::Subscribe<EventThread>{notifier}
        , notifier{notifier}
    {}
    ~SubscriberWithDispatchOnEvent() { REQUIRE(isCalled); }
    void onEvent(const EventThread& event)
    {
        REQUIRE(event.isCalledInSenderThread());
        isCalled = true;
        REQUIRE_FALSE(notifier.dispatch());
    }

private:
    easy::Notifier& notifier;
    bool isCalled = false;
};

class SubscriberTalkToItself : public easy::Subscribe<EventThread>
{
public:
    SubscriberTalkToItself(easy::Notifier& notifier, unsigned expectedCalledTimes)
        : easy::Subscribe<EventThread>{notifier}
        , notifier{notifier}
        , expectedCalledTimes{expectedCalledTimes}
    {}
    ~SubscriberTalkToItself() { REQUIRE(calledTimes == expectedCalledTimes); }

    void onEvent(const EventThread& event)
    {
        REQUIRE(event.isCalledInSenderThread());
        if (calledTimes < expectedCalledTimes)
        {
            ++calledTimes;
            notifier.publish(EventThread());
        }
    }

private:
    easy::Notifier& notifier;
    unsigned calledTimes = 0;
    unsigned expectedCalledTimes = 0;
};

template <typename T>
class Subscriber : public easy::Subscribe<T>
{
public:
    Subscriber(easy::Notifier& notifier) : easy::Subscribe<T>{notifier} {}
    ~Subscriber() { REQUIRE(isCalled); }
    void onEvent(const T& event) { REQUIRE(event.isCalledInSenderThread()); isCalled = true; }

private:
    bool isCalled = false;
};

class MultiSubscriber : public easy::Subscribe<EventThread, EventThread2, EventThread3>
{
public:
    MultiSubscriber(easy::Notifier& notifier, unsigned expectedCalledTimes)
        : easy::Subscribe<EventThread, EventThread2, EventThread3>{notifier}
        , expectedCalledTimes{expectedCalledTimes}
    {}
    ~MultiSubscriber() { REQUIRE(calledTimes == expectedCalledTimes); }
    void onEvent(const EventThread& event) { REQUIRE(event.isCalledInSenderThread()); ++calledTimes; }
    void onEvent(const EventThread2& event) { REQUIRE(event.isCalledInSenderThread()); ++calledTimes; }
    void onEvent(const EventThread3& event) { REQUIRE(event.isCalledInSenderThread()); ++calledTimes; }

private:
    unsigned calledTimes = 0;
    unsigned expectedCalledTimes = 0;
};


TEST_CASE("Nothing to dispatch if no event sent", "[single_thread][single_notifier]")
{
    easy::Notifier notifier;
    auto sub = NeverCalledSubscriber<EventThread>(notifier);
    REQUIRE_FALSE(notifier.dispatch());
};

TEST_CASE("Subscriber does not send event to itself", "[single_thread][single_notifier]")
{
    easy::Notifier notifier;
    auto sub = NeverCalledSubscriber<EventThread>(notifier);
    notifier.dispatch();
    notifier.publish(EventThread());
    REQUIRE_FALSE(notifier.dispatch());
};

TEST_CASE("Subscriber does not send event to others in the same notifier", "[single_thread][single_notifier]")
{
    easy::Notifier notifier;
    auto sub = NeverCalledSubscriber<EventThread>(notifier);
    auto sub2 = NeverCalledSubscriber<EventThread>(notifier);
    notifier.dispatch();
    notifier.publish(EventThread());
    REQUIRE_FALSE(notifier.dispatch());
};

TEST_CASE("Event is send between notifiers in the same thread", "[single_thread][multiple_notifiers]")
{
    easy::Notifier neverCalledSubscriberNotifier;
    easy::Notifier notifier;

    auto sub = NeverCalledSubscriber<EventThread>(neverCalledSubscriberNotifier);
    auto sub2 = Subscriber<EventThread>(notifier);

    neverCalledSubscriberNotifier.dispatch();
    notifier.dispatch();

    neverCalledSubscriberNotifier.publish(EventThread());

    REQUIRE_FALSE(neverCalledSubscriberNotifier.dispatch());
    REQUIRE(notifier.dispatch());

    REQUIRE_FALSE(neverCalledSubscriberNotifier.dispatch());
    REQUIRE_FALSE(notifier.dispatch());
};

TEST_CASE("Event is not sent to different subscribers for the same notifier", "[single_thread][multiple_notifiers]")
{
    easy::Notifier neverCalledSubscriberNotifier;

    auto sub = NeverCalledSubscriber<EventThread>(neverCalledSubscriberNotifier);
    auto sub2 = NeverCalledSubscriber<EventThread2>(neverCalledSubscriberNotifier);

    neverCalledSubscriberNotifier.dispatch();

    neverCalledSubscriberNotifier.publish(EventThread());
    neverCalledSubscriberNotifier.publish(EventThread2());

    REQUIRE_FALSE(neverCalledSubscriberNotifier.dispatch());
};

TEST_CASE("Subscribers under different notifiers send event with no problem", "[single_thread][multiple_notifiers]")
{
    easy::Notifier notifier;
    easy::Notifier notifier2;

    const auto EVENTS_SENT_TO_EACH_OTHER = 3u;
    auto sub = SubscriberTalkToItself(notifier, EVENTS_SENT_TO_EACH_OTHER);
    auto sub2 = SubscriberTalkToItself(notifier2, EVENTS_SENT_TO_EACH_OTHER);

    notifier.publish(EventThread());

    for (auto i = 0u; i < EVENTS_SENT_TO_EACH_OTHER; ++i)
    {
        /***
         * For each loop sub2(notifier2) dispatch Event sent from sub(notifier)
         * and send the Event back from sub2(notifier2) to sub(notifier) which dispatch own
         * event and send it back (sub2[dispatch and send back] -> sub[dispatch and send back])
         */
        REQUIRE(notifier2.dispatch());
        REQUIRE(notifier.dispatch());
    }
    REQUIRE(notifier2.dispatch());

    REQUIRE_FALSE(notifier.dispatch());
    REQUIRE_FALSE(notifier2.dispatch());
};

TEST_CASE("All subscribers receive subscribed event", "[single_thread][multiple_notifiers]")
{
    easy::Notifier notifier, notifier2, notifier3, notifier4;

    auto sub = NeverCalledSubscriber<EventThread>(notifier);
    auto sub2 = Subscriber<EventThread2>(notifier2);
    auto sub3 = Subscriber<EventThread2>(notifier3);
    auto sub4 = Subscriber<EventThread2>(notifier4);

    notifier.publish(EventThread2());

    REQUIRE_FALSE(notifier.dispatch());
    REQUIRE(notifier2.dispatch());
    REQUIRE(notifier3.dispatch());
    REQUIRE(notifier4.dispatch());

    REQUIRE_FALSE(notifier.dispatch());
    REQUIRE_FALSE(notifier2.dispatch());
    REQUIRE_FALSE(notifier3.dispatch());
    REQUIRE_FALSE(notifier4.dispatch());
};

TEST_CASE("Subscribers under different notifiers receives only subscribed events", "[single_thread][multiple_notifiers]")
{
    easy::Notifier notifier, notifier2, notifier3, notifier4;

    auto sub = Subscriber<EventThread>(notifier);
    auto sub2 = Subscriber<EventThread2>(notifier2);
    auto sub3 = Subscriber<EventThread3>(notifier3);
    auto sub4 = Subscriber<EventThread4>(notifier4);

    notifier.publish(EventThread2());
    notifier2.publish(EventThread3());
    notifier3.publish(EventThread4());
    notifier4.publish(EventThread());

    REQUIRE(notifier.dispatch());
    REQUIRE(notifier2.dispatch());
    REQUIRE(notifier3.dispatch());
    REQUIRE(notifier4.dispatch());

    REQUIRE_FALSE(notifier.dispatch());
    REQUIRE_FALSE(notifier2.dispatch());
    REQUIRE_FALSE(notifier3.dispatch());
    REQUIRE_FALSE(notifier4.dispatch());
};

TEST_CASE("Subscriber will block attempt to recursive dispatch", "[single_thread][single_notifier]")
{
    easy::Notifier notifierBase;
    easy::Notifier notifier;

    auto sub = SubscriberWithDispatchOnEvent(notifier);

    notifierBase.publish(EventThread());

    REQUIRE(notifier.dispatch());
    REQUIRE_FALSE(notifierBase.dispatch());
};

TEST_CASE("Subscriber for multiple events will receive only those", "[single_thread][single_notifier]")
{
    easy::Notifier notifierBase;
    easy::Notifier notifier;

    const auto EVENTS_RECEIVED_EXPECTED = 3u;

    auto sub = MultiSubscriber(notifier, EVENTS_RECEIVED_EXPECTED);

    notifierBase.publish(EventThread4());
    notifierBase.publish(EventThread4());
    notifierBase.publish(EventThread4());

    REQUIRE_FALSE(notifier.dispatch());

    notifierBase.publish(EventThread3());
    notifierBase.publish(EventThread4());
    notifierBase.publish(EventThread());
    notifierBase.publish(EventThread2());

    REQUIRE(notifier.dispatch());
    REQUIRE(notifier.dispatch());
    REQUIRE(notifier.dispatch());
    REQUIRE_FALSE(notifier.dispatch());
};

}  // namespace single_thread
