//
// Created by Jonah on 9/11/2021.
//

#include "Profiler.h"
#include <iostream>

Profiler* Profiler::instance{nullptr};
std::mutex Profiler::mutex_;

Profiler::Profiler() : timers() {}

Profiler *Profiler::getInstance() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (instance == nullptr) {
        instance = new Profiler();
    }
    return instance;
}

void Profiler::start(std::string name) {
    if (!timers.contains(name)) {
        auto newTimer = TimeAverage();
        newTimer.index = 0;
        newTimer.samplesStored = 0;
        timers[name] = newTimer;
    }

    auto& timer = timers[name];
    timer.startTime = std::chrono::high_resolution_clock::now();
}

void Profiler::end(std::string name) {
    auto endTime = std::chrono::high_resolution_clock::now();
    auto& timer = timers[name];
    timer.times[timer.index] = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - timer.startTime);
    timer.index++;
    if (timer.samplesStored < timer.index) timer.samplesStored = timer.index;
    timer.index %= samples;
}

void Profiler::dump(std::string filename) {
    for (const auto &item : timers) {

    }
}

void Profiler::print() {
    for (const auto &item : timers) {
        std::cout << item.first << ": " << calculateAverage(item.second) << std::endl;
    }
}

double Profiler::calculateAverage(const Profiler::TimeAverage &timeAverage) const {
    unsigned long long acc = 0; // uhh
    for (int i = 0; i < timeAverage.samplesStored; ++i) {
        acc += timeAverage.times[i].count();
    }
    return acc / (double) timeAverage.samplesStored;
}
