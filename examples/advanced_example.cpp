#include <bits/stdc++.h>

#include "../easy/subscriber.hpp"
#include "../easy/notifier.hpp"


namespace
{

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


class SensorObserver : public easy::Subscribe<DemandSensorDataEvent, StopObserverEvent>
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

}  // namespace


namespace examples
{

void advanced_example()
{
    using std::literals::chrono_literals::operator""ms;

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

}  // namespace
