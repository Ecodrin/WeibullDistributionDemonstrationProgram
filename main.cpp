#include <iostream>
#include <thread>
#include <chrono>

#include "weibull_distribution.hpp"

int main() {
    weibull_distribution_math::weibull_distribution w(0.5, 1);
    for (int i = 0; i < 5; i++) {
        std::cout << w.calc_membership(double(i) / 10.0) << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::seconds(4));
}