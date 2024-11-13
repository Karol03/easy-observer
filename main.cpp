/**
 * Created by Karol Dudzic @ 2022
 */
#include <bits/stdc++.h>

#include "easy/subscriber.hpp"
#include "easy/notifier.hpp"


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


class NeverCalledSubscriber : public easy::Subscribe<EventThread>
{
public:
    NeverCalledSubscriber(easy::Notifier& notifier) : easy::Subscribe<EventThread>{notifier} {}
    ~NeverCalledSubscriber() { }
    void onEvent(const EventThread& event) { isCalled = true; }

private:
    bool isCalled = false;
};

class Subscriber : public easy::Subscribe<EventThread>
{
public:
    Subscriber(easy::Notifier& notifier) : easy::Subscribe<EventThread>{notifier} {}
    // ~Subscriber() { REQUIRE(isCalled); }
    void onEvent(const EventThread& event) { isCalled = true; }

private:
    bool isCalled = false;
};

int main()
{
    easy::Notifier neverCalledSubscriberNotifier;
    easy::Notifier notifier;

    auto sub = NeverCalledSubscriber(neverCalledSubscriberNotifier);
    auto sub2 = Subscriber(notifier);

    neverCalledSubscriberNotifier.dispatch();
    notifier.dispatch();

    neverCalledSubscriberNotifier.publish(EventThread());

    neverCalledSubscriberNotifier.dispatch();
    notifier.dispatch();

}
