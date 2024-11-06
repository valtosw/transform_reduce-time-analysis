#pragma once

#include <iostream>
#include <chrono>

using std::cout, std::endl;
using std::chrono::high_resolution_clock, std::chrono::duration_cast, std::chrono::microseconds;

template<typename Func>
auto measure_time(Func&& func) {
    auto start = high_resolution_clock::now();
    func();
    auto end = high_resolution_clock::now();

    return duration_cast<microseconds>(end - start).count();
}