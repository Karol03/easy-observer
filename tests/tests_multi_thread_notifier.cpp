#include "catch2/catch_amalgamated.hpp"
#include "easy/subscriber.hpp"
#include "easy/notifier.hpp"

#include <thread>


namespace multi_thread
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

class NeverCalledSubscriber : public easy::Subscribe<EventThread>
{
public:
    NeverCalledSubscriber(easy::Notifier& notifier) : easy::Subscribe<EventThread>{notifier} {}
    ~NeverCalledSubscriber() { REQUIRE_FALSE(isCalled); }
    void onEvent(const EventThread& event) { REQUIRE_FALSE(event.isCalledInSenderThread()); isCalled = true; }

private:
    bool isCalled = false;
};

class NeverCalledSubscriber2 : public easy::Subscribe<EventThread2>
{
public:
    NeverCalledSubscriber2(easy::Notifier& notifier) : easy::Subscribe<EventThread2>{notifier} {}
    ~NeverCalledSubscriber2() { REQUIRE_FALSE(isCalled); }
    void onEvent(const EventThread2& event) { REQUIRE_FALSE(event.isCalledInSenderThread()); isCalled = true; }

private:
    bool isCalled = false;
};

class Subscriber : public easy::Subscribe<EventThread>
{
public:
    Subscriber(easy::Notifier& notifier) : easy::Subscribe<EventThread>{notifier} {}
    ~Subscriber() { REQUIRE(isCalled); }
    void onEvent(const EventThread& event)
    {
        REQUIRE_FALSE(event.isCalledInSenderThread());
        isCalled = true;
    }

private:
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
        REQUIRE_FALSE(event.isCalledInSenderThread());
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
class SubscriberT : public easy::Subscribe<T>
{
public:
    SubscriberT(easy::Notifier& notifier) : easy::Subscribe<T>{notifier} {}
    ~SubscriberT() { REQUIRE(isCalled); }
    void onEvent(const T& event) { REQUIRE_FALSE(event.isCalledInSenderThread()); isCalled = true; }

private:
    bool isCalled = false;
};


TEST_CASE("Subscriber rceives event from different thread", "[multiple_threads][single_notifier]")
{
    using std::literals::chrono_literals::operator""ms;

    easy::Notifier notifier;

    auto t1 = std::thread([]() {
        const auto timeout = 50ms;
        easy::Notifier notifier;
        auto sub = Subscriber(notifier);
        auto start = std::chrono::high_resolution_clock::now();
        while (!notifier.dispatch())
        {
            if (std::chrono::high_resolution_clock::now() - start >= timeout)
            {
                return;
            }
        }
        REQUIRE_FALSE(notifier.dispatch());
    });

    std::this_thread::sleep_for(10ms);  // run second thread to initialize and subscribe for event
    notifier.publish(EventThread());

    if (t1.joinable())
        t1.join();

    REQUIRE_FALSE(notifier.dispatch());
};

TEST_CASE("Subscribers under different notifiers send event with no problem", "[multiple_threads][multiple_notifiers]")
{
    using std::literals::chrono_literals::operator""ms;

    easy::Notifier notifier;

    auto subThread = []() {
        const auto EVENTS_SENT_TO_EACH_OTHER = 3u;
        auto dispatchTimes = 0;
        const auto timeout = 200ms;
        easy::Notifier notifier;
        auto sub = SubscriberTalkToItself(notifier, EVENTS_SENT_TO_EACH_OTHER - 1);
        auto start = std::chrono::high_resolution_clock::now();
        while (dispatchTimes != EVENTS_SENT_TO_EACH_OTHER)
        {
            dispatchTimes += notifier.dispatch();
            if (std::chrono::high_resolution_clock::now() - start >= timeout)
            {
                return;
            }
        }
        REQUIRE_FALSE(notifier.dispatch());
    };

    auto t1 = std::thread(subThread);
    auto t2 = std::thread(subThread);

    std::this_thread::sleep_for(20ms);  // run second thread to initialize and subscribe for event
    notifier.publish(EventThread());

    if (t1.joinable())
        t1.join();

    if (t2.joinable())
        t2.join();

    REQUIRE_FALSE(notifier.dispatch());
};

TEST_CASE("All subscribers receive subscribed event", "[multiple_threads][multiple_notifiers]")
{
    using std::literals::chrono_literals::operator""ms;

    easy::Notifier notifier;

    auto subThread = []() {
        const auto timeout = 200ms;
        easy::Notifier notifier;
        auto sub = Subscriber(notifier);
        auto start = std::chrono::high_resolution_clock::now();
        while (!notifier.dispatch())
        {
            if (std::chrono::high_resolution_clock::now() - start >= timeout)
            {
                return;
            }
        }
        REQUIRE_FALSE(notifier.dispatch());
    };

    auto t1 = std::thread(subThread);
    auto t2 = std::thread(subThread);
    auto t3 = std::thread(subThread);
    auto t4 = std::thread(subThread);

    std::this_thread::sleep_for(50ms);  // run second thread to initialize and subscribe for event
    notifier.publish(EventThread());

    if (t1.joinable())
        t1.join();

    if (t2.joinable())
        t2.join();

    if (t3.joinable())
        t3.join();

    if (t4.joinable())
        t4.join();

    REQUIRE_FALSE(notifier.dispatch());
};

TEST_CASE("Subscribers under different notifiers receives only subscribed events", "[multiple_threads][multiple_notifiers]")
{
    using std::literals::chrono_literals::operator""ms;

    easy::Notifier notifier, notifier2, notifier3, notifier4;

    auto subThread = []() {
        const auto timeout = 200ms;
        easy::Notifier notifier;
        auto sub = SubscriberT<EventThread>(notifier);
        auto start = std::chrono::high_resolution_clock::now();
        while (!notifier.dispatch())
        {
            if (std::chrono::high_resolution_clock::now() - start >= timeout)
            {
                return;
            }
        }
        notifier.publish(EventThread2());
        REQUIRE_FALSE(notifier.dispatch());
    };

    auto subThread2 = []() {
        const auto timeout = 200ms;
        easy::Notifier notifier;
        auto sub = SubscriberT<EventThread2>(notifier);
        auto start = std::chrono::high_resolution_clock::now();
        while (!notifier.dispatch())
        {
            if (std::chrono::high_resolution_clock::now() - start >= timeout)
            {
                return;
            }
        }
        notifier.publish(EventThread3());
        REQUIRE_FALSE(notifier.dispatch());
    };

    auto subThread3 = []() {
        const auto timeout = 200ms;
        easy::Notifier notifier;
        auto sub = SubscriberT<EventThread3>(notifier);
        auto start = std::chrono::high_resolution_clock::now();
        while (!notifier.dispatch())
        {
            if (std::chrono::high_resolution_clock::now() - start >= timeout)
            {
                return;
            }
        }
        notifier.publish(EventThread4());
        REQUIRE_FALSE(notifier.dispatch());
    };

    auto subThread4 = []() {
        const auto timeout = 200ms;
        easy::Notifier notifier;
        auto sub = SubscriberT<EventThread4>(notifier);
        auto start = std::chrono::high_resolution_clock::now();
        while (!notifier.dispatch())
        {
            if (std::chrono::high_resolution_clock::now() - start >= timeout)
            {
                return;
            }
        }
        REQUIRE_FALSE(notifier.dispatch());
    };

    auto t1 = std::thread(subThread);
    auto t2 = std::thread(subThread2);
    auto t3 = std::thread(subThread3);
    auto t4 = std::thread(subThread4);

    std::this_thread::sleep_for(50ms);  // run second thread to initialize and subscribe for event
    notifier.publish(EventThread());

    if (t1.joinable())
        t1.join();

    if (t2.joinable())
        t2.join();

    if (t3.joinable())
        t3.join();

    if (t4.joinable())
        t4.join();

    REQUIRE_FALSE(notifier.dispatch());
};

}  // namespace multi_thread
