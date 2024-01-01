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
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> uniqueLock(_mutex);
    _ready.wait(uniqueLock, [this]{return !_queue.empty();});

    T msg = std::move(_queue.front());
    _queue.pop_front();

    return msg;

}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
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
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
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
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    // print id of the current thread

    // initalize variables
    double cycleDuration = 4; // duration of a single simulation in ms
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
            //set cycle duration
            std::random_device rd;
            std::default_random_engine generator(rd());
            // Generate a random number between 4 and 6
            std::uniform_real_distribution<double> distribution(4, 6);
            cycleDuration = distribution(generator);
            
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
