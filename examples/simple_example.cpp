#include <bits/stdc++.h>

#include "../easy/subscriber.hpp"
#include "../easy/notifier.hpp"


namespace
{

class InputEvent : public easy::Event<InputEvent>  // important to inherit from east::Event
{
public:
    InputEvent(std::string phrase)
        : phrase{phrase}
    {}

public:
    std::string phrase = {};
};


class OnCharPressedSubscriber : public easy::Subscribe<InputEvent>  // important to inherit from east::Subscribe with the observed events in the template
{
public:
    OnCharPressedSubscriber(easy::Notifier& notifier)  // takes notifier used by the subscriber
        : easy::Subscribe<InputEvent>{notifier}        // subscribes for events
    {}

private:
    void onEvent(const InputEvent& event)    // callback called when event happens
    {
        std::cout << "Received phrase: " << event.phrase << "\n";
        if (event.phrase == "q" || event.phrase == "quit")
        {
            isWorking = false;
            std::cout << "Bye!\n";
        }
    }

public:
    bool isWorking = true;
};

}  // namespace


namespace examples
{

void simple_example()
{
    auto notifier = easy::Notifier{};            // create notifier running in the main thread
    auto subscriberNotifier = easy::Notifier{};  // create notifier specified for our subscriber

    auto subscriber = OnCharPressedSubscriber(subscriberNotifier);

    std::string phrase;
    while (subscriber.isWorking)
    {
        std::cout << "Enter phrase ('q' or 'quit' to exit): \n";
        std::cin >> phrase;
        notifier.publish(InputEvent{phrase});    // publishing an event
        subscriberNotifier.dispatch();           // dispatch all events notifier is aware of to subscribers
    }
}

}  // namespace examples
