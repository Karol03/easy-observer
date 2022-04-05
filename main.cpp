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


/**
 * @brief Declare own event
 * Inherite from Event<> with match template
 *
 * Added some method to check if it call properly
 */
class Event_test : public eob::Event<Event_test>
{
public:
    Event_test(std::string name)
        : m_name{name}
    {}

    void print() const
    {
        std::cerr << "Hello world from Event_test named '" << m_name << "'\n";
    }

private:
    std::string m_name;
};


/**
 * @brief Declare own class
 * Class subscribes on Event_type only,
 * in case of more subscription types give them in parameter pack like:
 *      Subscriber<Event_test, Event_test_2, Event_test_3>
 */
class MyClass : private eob::Subscriber<Event_test>
{
public:
    /**
     * @brief MyClass
     * @param nt - reference to application thread which will collect and dispatch notification
     *             for the class
     */
    MyClass(eob::NotifyThread& nt)
        : Subscriber<Event_test>{nt}
    {}

protected:
    /**
     * @brief onEvent
     * @param event
     * Callback when event caught. Each observed event must have own callback
     */
    void onEvent(const Event_test& event) { event.print(); }
};


int main()
{
    eob::DispatcherThread mainThread;       // start main notification dispatcher - required,
                                            // only one dispatcher possible

    eob::ApplicationThread appThread;       // start application thread - required,
                                            // multiple threads allowed

    MyClass mc{appThread};                  // create own class with subscription

    // ...
    appThread.emit(Event_test{"My name"});  // simmulate somewhere in code some class emit event
                                            // in class you don't have to use appThread to emit event
                                            // subscriber has already built-in method to do this
    // ...

    std::this_thread::sleep_for(std::chrono_literals::operator""s(1));  // give some time to dispatch events

    return 0;
}
