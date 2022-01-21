#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> lck(_mutex);

    // wait till there is a new message
    _condition.wait(lck,[this]{ return !_queue.empty(); });

    // read the msg from the queue using move semantics
    T msg = std::move(_queue.back());
    
    // clear it from the queue
    _queue.clear();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lck(_mutex);

    _queue.emplace_back(std::move(msg));

    _condition.notify_one();
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

    while(true){
        TrafficLightPhase received = _msgQueue.receive();
        if(received == TrafficLightPhase::green){
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    std::thread thread(&TrafficLight::cycleThroughPhases, this);
    threads.emplace_back(std::move(thread));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    // generate random number between 4 and 6
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_real_distribution<> distr(4000.0,6000.0);
    double cycleTime = distr(eng);

    // start stop watch
    auto pastTime = std::chrono::system_clock::now();

    while(true){

        // wait 1ms between cycles to reduce CPU load
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // calculate cycle time 
        auto currTime = std::chrono::system_clock::now();
        double timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(currTime - pastTime).count();

        
        if(timeDiff >= cycleTime){

            // toggle traffic light
            _currentPhase = (_currentPhase == TrafficLightPhase::green) ? TrafficLightPhase::red : TrafficLightPhase::green;

            _msgQueue.send(std::move(_currentPhase));

            // reset stop watch
            pastTime = std::chrono::system_clock::now();
        }

    }
}

