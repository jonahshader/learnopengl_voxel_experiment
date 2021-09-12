//
// Created by Jonah on 9/11/2021.
//

#ifndef LEARNOPENGL_VOXEL_EXPERIMENT_PROFILER_H
#define LEARNOPENGL_VOXEL_EXPERIMENT_PROFILER_H

#include <chrono>
#include <unordered_map>
#include <string>
#include <mutex>
#include <vector>

class Profiler {
private:
    // TODO: make struct with boolean to keep track of start end
    static constexpr int samples = 32;
    struct TimeAverage {
        std::chrono::high_resolution_clock::duration times[samples];
        std::chrono::high_resolution_clock::time_point startTime;
        int index;
        int samplesStored;
    };

    Profiler();

    static Profiler* instance;
    static std::mutex mutex_;

    std::unordered_map<std::string, TimeAverage> timers;

    double calculateAverage(const TimeAverage &timeAverage) const;

public:
    Profiler(Profiler &other) = delete;
    void operator=(const Profiler&) = delete;
    static Profiler *getInstance();

    void start(std::string name);
    void end(std::string name);
    void dump(std::string filename);
    void print();

};


#endif //LEARNOPENGL_VOXEL_EXPERIMENT_PROFILER_H
