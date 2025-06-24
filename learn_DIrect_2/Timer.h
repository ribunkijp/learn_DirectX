/*
    Timer.h

*/
#ifndef TIMER_H
#define TIMER_H

#include <chrono>

class Timer {
public:
    Timer();

    void Reset();
    void Tick();
    float GetDeltaTime() const;

private:
    float deltaTime;
    //
    char padding[4] = {};

    std::chrono::high_resolution_clock::time_point lastTime;
};





#endif
