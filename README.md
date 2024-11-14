# easy-pubsub
Easy-pubsub is small library with no external dependencies meet subscribe-publish pattern. The library allows in-process communication between different instances of objects, and between threads. <br/>
It will find its use in single/multi-threaded applications where we transfer data between separate parts of the software while running in a loop (e.g. game loop, reading and processing data). With the publish/subscribe model, we can keep the code clean by separating the different logical parts. <br/>

# Demonstration
## A simple example
In this example we create single-thread application. We will take the phrase entered by the user and send it to subscribers. <br/> <br/>

First, we create an event that will be sent to subscribers. It is important that our event inherits from the Event class found in the event.hpp file. This class is a template, base class. All events must inherit from it and meet CRTP (recurring template pattern) <br/>
```cpp
class InputEvent : public easy::Event<InputEvent>  // important to inherit from east::Event
{
public:
    InputEvent(std::string phrase)
        : phrase{phrase}
    {}

public:
    std::string phrase = {};
};
```
Our event contains one field with a phrase, the number and type of fields is not limited, but remember to properly manage the life cycle of objects. The event class does not take ownership over the objects, and the objects are copied when passing. Dynamically allocated memory should be released correctly. <br/> <br>

Next, we create a subscriber for our event. The subscriber must inherit from the easy::Subscribe class, to which we specify in the template all the events we would like to observe. Our subscriber must initialize the base class in the constructor using a reference to the easy::Notifier object. This is required because messages are sent between notifiers (this means that two subscribers using the same notifier will not see each other's published events), however, we can assign any number of subscribers for a single notifier. <br/>
For each event subsumed by the class, reload the onEvent(...) method. This method will be called when the event is happened. In our case, this is one method for the InputEvent event. <br/><br/>
```cpp
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
```

All the necessary structures are ready now we create the main part of the program.<br/>
As explained above, event information is sent between notifiers, so we need to use two notifiers. One, to which we will assign a subscriber, and another, which will publish the event. <br/>
```cpp
int main()
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
```

## A more complex example
The example shows two subscribers in two separate threads. One requires sensor data, the other publishes the data. <br/>
At the beginning, we prepare the relevant events. </br>
- StopObserverEvent - An event that informs the subscriber responsible for reading the data that no more data is needed and the work can be terminated 
- DemandSensorDataEvent - An event that informs the subscriber responsible for reading the data that another portion of data is needed
- SensorReadEvent - Event notifying subscriber requiring data of new portion of data
```cpp
class StopObserverEvent : public easy::Event<StopObserverEvent>     // event indicates that SensorObserver should stop the action
{};

class DemandSensorDataEvent : public easy::Event<DemandSensorDataEvent>  // event indicates that SensorValueReceiver need another value from SensorObserver
{};

class SensorReadEvent : public easy::Event<SensorReadEvent>  // event with new value read by SensorObserver
{
public:
    SensorReadEvent(int data)
        : data{data}
    {}

public:
    int data = {};
};
```

We create a subscriber who will be responsible for reading the data. It is subscribed to two events: 
- Data request (DemandSensorDataEvent)
- End of activity (StopObserverEvent)
```cpp
class SensorObserver : public easy::Subscribe<DemandSensorDataEvent, StopObserverEvent>  // subscribed for both events
{
public:
    SensorObserver(easy::Notifier& notifier)
        : easy::Subscribe<DemandSensorDataEvent, StopObserverEvent>{notifier}
        , notifier{notifier}
    {}

private:
    void onEvent(const DemandSensorDataEvent& event)
    {
        const auto readValue = rand() % 1024;
        notifier.publish(SensorReadEvent(readValue));
    }

    void onEvent(const StopObserverEvent& event)
    {
        isWorking = false;
    }

public:
    easy::Notifier& notifier;
    bool isWorking = true;
};
```

We create a subscriber for the second thread. This subscriber will process (for 1000ms = 1 second) the received data. As long as the data is not as expected (< threshold) it will ask for another portion of data. When the data agrees it will notify the observer that no more data is needed and will terminate the activity itself. 
```cpp
class SensorValueReceiver : public easy::Subscribe<SensorReadEvent>
{
public:
    SensorValueReceiver(easy::Notifier& notifier)
        : easy::Subscribe<SensorReadEvent>{notifier}
        , notifier{notifier}
    {}

    void checkData()
    {
        const int requiredDataThreshold = 512;
        std::cout << "Current value is {" << lastData << "}, required >" << requiredDataThreshold;
        if (lastData < requiredDataThreshold)    // read values untill lower than threshold
        {
            std::cout << " - demand next value\n";
            notifier.publish(DemandSensorDataEvent());
        }
        else
        {
            std::cout << "- condition satified send event to stop observer\n";
            notifier.publish(StopObserverEvent());
            isWorking = false;
        }
    }

private:
    void onEvent(const SensorReadEvent& event)
    {
        using std::literals::chrono_literals::operator""ms;
        std::this_thread::sleep_for(1000ms);
        lastData = event.data;
        checkData();
    }

public:
    easy::Notifier& notifier;
    int lastData = -1;
    bool isWorking = true;
};
```

Main function, we create two threads. In both we dispatch received events for until object to report to end of the work.
```cpp
int main()
{
    auto t1 = std::thread([](){
        auto notifier = easy::Notifier{};
        auto observer = SensorObserver(notifier);
        while (observer.isWorking)
            notifier.dispatch();
    });

    auto t2 = std::thread([](){
        auto notifier = easy::Notifier{};
        auto valueReceiver = SensorValueReceiver(notifier);
        valueReceiver.checkData();
        while (valueReceiver.isWorking)
            notifier.dispatch();
    });

    if (t1.joinable())
        t1.join();

    if (t2.joinable())
        t2.join();
}
```

<br/><br/>Both complete examples are available in examples/ directory.

# Tests
TestCases written in [catch2](https://github.com/catchorg/Catch2).
Amalgamated version of library added to repository to simplify testing by skipping the installation of the full framework.

# Requirements
C++20 (compiled with MinGw 11.2)
