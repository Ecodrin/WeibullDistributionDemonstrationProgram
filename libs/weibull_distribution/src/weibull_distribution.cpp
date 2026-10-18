#include "weibull_distribution.hpp"

namespace weibull_distribution_math {
    weibull_distribution::weibull_distribution(double k, double lambda) : k{k}, lambda{lambda} {

    }

    double weibull_distribution::probability_density_function(double x) {
        if (x < 0) {
            return 0;
        }
        return (k / lambda) * std::pow(x / lambda, k-1) * std::exp(-std::pow(x / lambda, k));
    }

    double weibull_distribution::membership_function(double x) {
        if (k > 1) {
            double x_max = lambda * std::pow((k-1) / k, 1/k);
            return std::pow(x / x_max, k-1) * std::exp(std::pow(x_max / lambda, k) - std::pow(x / lambda, k));
        } else {
            return 1 - std::exp(-std::pow(x / lambda, k));
        }
    }
    double weibull_distribution::calc_membership(double x) {
        return membership_function(x);
    }
}