/**
 * Created by Karol Dudzic @ 2022
 */
#include <bits/stdc++.h>

/**
 * @brief
 * "subscriber.hpp" header for subscriber class
 * "threadwrapper.hpp" for thread classes  -
 *      it is possible to use classes from "thread.hpp" in 'raw' version
 *      but it is unrecommended as threadwrappers already manage thread lifetime
 */
#include "eob/subscriber.hpp"
#include "eob/threadwrapper.hpp"


// ========================== CUSTOM EVENT DECLARATION ================================
/**
 * @brief Declare two events
 * Each event must inherit from Event<> with own typename
 *      it is required to proper register event type
 *
 * Added a method to check if it will call properly from event handler
 */
class Event_test : public eob::Event<Event_test>
{
public:
    Event_test(std::string name)
        : m_name{name}
    {}

    void print() const
    {
        std::cerr << "Event_test named '" << m_name << "' print\n";
    }

private:
    std::string m_name;
};


class Event_test_2 : public eob::Event<Event_test_2>
{
public:
    Event_test_2(std::string name)
        : m_name{name}
    {}

    void print() const
    {
        std::cerr << "Event_test_2 named '" << m_name << "' print\n";
    }

private:
    std::string m_name;
};

// ========================== CUSTOM EVENT DECLARATION ================================


// ========================== CLASSES DECLARATION ================================
/**
 * @brief Declare own classes
 * MyClass subscribes on Event_type only, when MyClass2 on Event_type_2 only,
 * It is possible to subscribe on more event types, in this case inherit from Subscriber
 *      using parameter pack like: Subscriber<Event_test, Event_test_2, Event_test_3>
 */
class MyClass : private eob::Subscriber<Event_test>
{
public:
    /**
     * @brief MyClass
     * @param notifier - reference to application notifier which will collect and dispatch
     *                   notification for the class
     */
    MyClass(eob::Notifier& notifier)
        : Subscriber<Event_test>{notifier}
    {}

    /**
     * @brief run
     * Method which emit some event
     */
    void run()
    {
        emit(Event_test_2("Sent from MyClass"));
    }

protected:
    /**
     * @brief onEvent
     * @param event
     * Callback when event 'Event_test' caught. Each observed event must override callback
     */
    void onEvent(const Event_test& event) override { event.print(); }
};


class MyClass2 : private eob::Subscriber<Event_test_2>
{
public:
    MyClass2(eob::Notifier& notifier)           // must take notifier
        : Subscriber<Event_test_2>{notifier}    // initialize subscriber with notifier
    {}

protected:
    void onEvent(const Event_test_2& event) override    // override onEvent callback
    {
        event.print();                                  // print event details
        emit(Event_test("Sent from MyClass2"));         // emit different event from callback body
    }
};

// ========================== CLASSES DECLARATION ================================


// ===================== APPLICATION THREADS MAIN DECLARATION =======================

void appThreadMain(eob::Notifier& notifier)
{
    using std::chrono_literals::operator""s;

    MyClass mc{notifier};                  // create own class with subscription on 'Event_test'
    std::this_thread::sleep_for(1s);       // wait to other thread initialization
    mc.run();                              // emit event 'Event_test_2' from MyClass body
    notifier.wait();                       // if we want to receive events in this thread
                                           // it is required to keep notification loop alive
}

void appThreadMain2(eob::Notifier& notifier)
{
    MyClass2 mc{notifier};                 // create own class with subscription on 'Event_test_2'
    notifier.wait();                       // as above, keep notification loop
}

// ===================== APPLICATION THREADS MAIN DECLARATION =======================


int main()
{
    using std::chrono_literals::operator""ms;
    using std::chrono_literals::operator""s;

    {
        std::cerr << "First approach, keep alive threads by yourself\n";
        auto supervisor = eob::Supervisor{};                                    // create supervisor
        eob::AppThread appThread = supervisor.createAppThread(appThreadMain);   // create thread with own main function
        eob::AppThread appThread2 = supervisor.createAppThread(appThreadMain2); // create second thread with own main function
        std::this_thread::sleep_for(2s);  // keep threads alive for some time
                                          // you could use e.g. while loop with own end condition
        // here supervisor and all application threads will be destroyed
    }

    std::cerr << "\nGive some time to close all threads\n";
    std::this_thread::sleep_for(500ms);

    {
        std::cerr << "\nSecond approach, request supervisor to keep alive threads until specified event received\n";
        auto supervisor = eob::Supervisor<Event_test>{};    // create a supervisor for which
                                                            // the destruction condition is to
                                                            // receive the event 'Event_test'
        eob::AppThread appThread = supervisor.createAppThread(appThreadMain);   // create thread with own main function
        eob::AppThread appThread2 = supervisor.createAppThread(appThreadMain2); // create second thread with own main function
        // here supervisor and all application threads will be alive until 'Event_test' emission
    }
    return 0;
}
