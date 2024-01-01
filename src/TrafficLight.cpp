#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */  

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a
    std::unique_lock<std::mutex> uniqueLock(_mutex);
    _ready.wait(uniqueLock, [this]{return !_queue.empty();});

    T msg = std::move(_queue.front());
    _queue.pop_front();

    return msg;

}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a
    std::lock_guard<std::mutex> lck(_mutex);
    _queue.push_back(std::move(msg));
    _ready.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b
    while (true)
    {
       TrafficLightPhase phase = _phasesQueue.receive();

       if (TrafficLightPhase::green == phase)
       {
            return;
       }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    std::lock_guard<std::mutex> lck(_mutex);
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}
int Calculate()
{
        //set cycle duration
    std::random_device rd;
    std::mt19937 generator(rd());
    // Generate a random number between 4 and 6
    std::uniform_int_distribution<int> distribution(4, 6);
    return distribution(generator);
}
// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a

    // initalize variables
    double cycleDuration = Calculate(); // duration of a single simulation in ms
    std::chrono::time_point<std::chrono::system_clock> lastUpdate;
    std::unique_lock<std::mutex> lck(_mtx);
    std::cout << "TrafficLight #" << _id << "::cycleThroughPhases: thread id = " << std::this_thread::get_id() << std::endl;
    lck.unlock();
    // init stop watch
    lastUpdate = std::chrono::system_clock::now();
    while(true)
    {
        // sleep for 1 ms to reduce CPU load
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        // get time difference
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - lastUpdate).count();

        // check if duration reached
        if (timeSinceLastUpdate >= cycleDuration)
        {
            // Calculate next cycle duration
            cycleDuration = Calculate();
            
            std::lock_guard<std::mutex> lck(_mutex);
            // switch between lights
            _currentPhase = (_currentPhase == TrafficLightPhase::green)? TrafficLightPhase::red : TrafficLightPhase::green;

            // send new phase to the queue
            _phasesQueue.send(std::move(_currentPhase));
            // reset last update
            lastUpdate = std::chrono::system_clock::now();
        }
        
    }
}
