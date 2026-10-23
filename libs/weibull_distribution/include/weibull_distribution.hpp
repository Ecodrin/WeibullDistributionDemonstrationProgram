#pragma once

#include <iostream>
#include <functional>
#include <cmath>

namespace weibull_distribution_math {
    class weibull_distribution {
    private:
        long double k, lambda;
        long double probability_density_function(long double x);
    public:
        weibull_distribution(long double k=1, long double lambda=0.5);
        long double membership_function(long double x);
        long double get_max_x() const;
    };

    long double dichotomy_method(long double a, long double b, std::function<long double(long double)> f, int max_steps=100, long double eps=0.01);
}