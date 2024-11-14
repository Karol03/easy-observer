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

template <typename T>
class Subscriber : public easy::Subscribe<T>
{
public:
    Subscriber(easy::Notifier& notifier,
               unsigned expectedCalledTimes = 1,
               unsigned expectedSentEvents = 0,
               bool assert = true)
        : easy::Subscribe<T>{notifier}
        , notifier{notifier}
        , calledTimes{}
        , expectedCalledTimes{expectedCalledTimes}
        , expectedSentEvents{expectedSentEvents}
        , assert{assert}
    {}
    ~Subscriber()
    {
        if (assert)
            REQUIRE(calledTimes == expectedCalledTimes);
    }
    void onEvent(const T& event)
    {
        if (assert)
            REQUIRE_FALSE(event.isCalledInSenderThread());
        ++calledTimes;
        if (expectedSentEvents > 0)
        {
            --expectedSentEvents;
            notifier.publish(T());
        }
    }

private:
    easy::Notifier& notifier;
    unsigned calledTimes;
    unsigned expectedCalledTimes;
    unsigned expectedSentEvents;
    bool assert;
};

template <typename T, typename V>
class DoubleSubscriber : public easy::Subscribe<T, V>
{
public:
    DoubleSubscriber(easy::Notifier& notifier,
                     unsigned expectedCalledTimes = 1,
                     unsigned expectedSentEvents = 0,
                     bool assert = true)
        : easy::Subscribe<T, V>{notifier}
        , notifier{notifier}
        , calledTimes{}
        , expectedCalledTimes{expectedCalledTimes}
        , expectedSentEvents{expectedSentEvents}
        , assert{assert}
    {}
    ~DoubleSubscriber()
    {
        if (assert)
            REQUIRE(calledTimes == expectedCalledTimes);
    }
    void onEvent(const T& event)
    {
        if (assert)
            REQUIRE_FALSE(event.isCalledInSenderThread());
        ++calledTimes;
        if (expectedSentEvents > 0)
        {
            --expectedSentEvents;
            notifier.publish(T());
        }
    }

    void onEvent(const V& event)
    {
        notifier.publish(T());
    }

private:
    easy::Notifier& notifier;
    unsigned calledTimes;
    unsigned expectedCalledTimes;
    unsigned expectedSentEvents;
    bool assert;
};


TEST_CASE("Subscriber rceives event from different thread", "[multiple_threads][single_notifier]")
{
    using std::literals::chrono_literals::operator""ms;

    easy::Notifier notifier;

    auto t1 = std::thread([]() {
        const auto timeout = 50ms;
        easy::Notifier notifier;
        auto sub = Subscriber<EventThread>(notifier);
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
        auto sub = Subscriber<EventThread>(notifier,
                                            EVENTS_SENT_TO_EACH_OTHER,
                                            EVENTS_SENT_TO_EACH_OTHER - 1);
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
        auto sub = Subscriber<EventThread>(notifier);
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
        auto sub = Subscriber<EventThread>(notifier);
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
        auto sub = Subscriber<EventThread2>(notifier);
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
        auto sub = Subscriber<EventThread3>(notifier);
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
        auto sub = Subscriber<EventThread4>(notifier);
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


TEST_CASE("Multithread communication benchmark", "[multiple_threads][multiple_notifiers][benchmark]")
{
    using std::literals::chrono_literals::operator""ms;
    using std::literals::chrono_literals::operator""s;

    BENCHMARK_ADVANCED("2 Threads | 1 Notifier per Thread | 10 messages")(Catch::Benchmark::Chronometer meter) {
        const auto EVENTS_SENT_TO_EACH_OTHER = 10u;
        std::atomic_uint dispatchTimes1 = {};
        std::atomic_uint dispatchTimes2 = {};

        auto subThread = [&dispatchTimes1]() {
            const auto timeout = 30s;
            easy::Notifier notifier;
            auto sub = Subscriber<EventThread>(notifier,
                                               EVENTS_SENT_TO_EACH_OTHER,
                                               EVENTS_SENT_TO_EACH_OTHER,
                                               false);
            auto start = std::chrono::high_resolution_clock::now();
            while (dispatchTimes1 < EVENTS_SENT_TO_EACH_OTHER)
            {
                dispatchTimes1.fetch_add(notifier.dispatch());
                const auto spentTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start);
                if (spentTime >= timeout)
                {
                    return;
                }
            }
        };

        auto subThread2 = [&dispatchTimes2]() {
            const auto timeout = 30s;
            easy::Notifier notifier;
            auto sub = DoubleSubscriber<EventThread, EventThread2>(notifier,
                                                                   EVENTS_SENT_TO_EACH_OTHER,
                                                                   EVENTS_SENT_TO_EACH_OTHER,
                                                                   false);
            auto start = std::chrono::high_resolution_clock::now();
            while (dispatchTimes2 < EVENTS_SENT_TO_EACH_OTHER)
            {
                dispatchTimes2.fetch_add(notifier.dispatch());
                const auto spentTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start);
                if (spentTime >= timeout)
                {
                    return;
                }
            }
        };

        auto t1 = std::thread(subThread);
        auto t2 = std::thread(subThread2);

        easy::Notifier notifier;
        meter.measure([&notifier] {
            notifier.publish(EventThread2());
        });

        INFO("Dispatch error. Both threads should exchange " << EVENTS_SENT_TO_EACH_OTHER
                                                             << " events, but thread 1 received only "
                                                             << dispatchTimes1 << ", while thread 2 " << dispatchTimes2);
        CHECK((dispatchTimes1 == dispatchTimes2 && dispatchTimes1 == EVENTS_SENT_TO_EACH_OTHER));

        if (t1.joinable())
            t1.join();

        if (t2.joinable())
            t2.join();
    };

    // BENCHMARK_ADVANCED("2 Threads | 4 Notifier per Thread | 100 messages")(Catch::Benchmark::Chronometer meter) {
    //     ProductProxy<16u> proxy;
    //     for (auto id = 1; id <= 10; ++id)
    //         proxy.fetch(id);
    //     auto threadMain = [&proxy]() {
    //         for (auto id = 1u; id <= 10; ++id)
    //         {
    //             auto object = proxy.fetch((rand() % 10) + 1);
    //             (void)(object);
    //         }
    //     };

    //     meter.measure([&threadMain] {
    //         auto t1 = std::thread(threadMain);
    //         auto t2 = std::thread(threadMain);
    //         auto t3 = std::thread(threadMain);

    //         t1.join();
    //         t2.join();
    //         t3.join();
    //     });
    // };

    // BENCHMARK_ADVANCED("6 Threads | 1 Notifier per Thread | 100 messages")(Catch::Benchmark::Chronometer meter) {
    //     ProductProxy<16u> proxy;
    //     for (auto id = 1; id <= 10; ++id)
    //         proxy.fetch(id);
    //     auto threadMain = [&proxy]() {
    //         for (auto id = 1u; id <= 10; ++id)
    //         {
    //             auto object = proxy.fetch((rand() % 10) + 1);
    //             (void)(object);
    //         }
    //     };

    //     meter.measure([&threadMain] {
    //         auto t1 = std::thread(threadMain);
    //         auto t2 = std::thread(threadMain);
    //         auto t3 = std::thread(threadMain);

    //         t1.join();
    //         t2.join();
    //         t3.join();
    //     });
    // };

    // BENCHMARK_ADVANCED("6 Threads | 4 Notifier per Thread | 100 messages")(Catch::Benchmark::Chronometer meter) {
    //     ProductProxy<16u> proxy;
    //     for (auto id = 1; id <= 10; ++id)
    //         proxy.fetch(id);
    //     auto threadMain = [&proxy]() {
    //         for (auto id = 1u; id <= 10; ++id)
    //         {
    //             auto object = proxy.fetch((rand() % 10) + 1);
    //             (void)(object);
    //         }
    //     };

    //     meter.measure([&threadMain] {
    //         auto t1 = std::thread(threadMain);
    //         auto t2 = std::thread(threadMain);
    //         auto t3 = std::thread(threadMain);

    //         t1.join();
    //         t2.join();
    //         t3.join();
    //     });
    // };

}

}  // namespace multi_thread
