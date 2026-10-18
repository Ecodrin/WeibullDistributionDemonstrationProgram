#pragma once

#include <iostream>
#include <functional>
#include <cmath>

namespace weibull_distribution_math {
    class weibull_distribution {
    private:
        double k, lambda;
        double probability_density_function(double x);
        double membership_function(double x);
    public:
        weibull_distribution(double k=1, double lambda=0.5);
        double calc_membership(double x);
    };
}