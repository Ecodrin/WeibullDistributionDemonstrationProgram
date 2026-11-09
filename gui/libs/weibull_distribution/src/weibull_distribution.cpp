#include "weibull_distribution.hpp"

#include <algorithm>

namespace weibull_distribution_math {
    weibull_distribution::weibull_distribution(long double k, long double lambda) : k{k}, lambda{lambda} {

    }

    long double weibull_distribution::probability_density_function(long double x) {
        if (x < 0) {
            return 0;
        }
        return (k / lambda) * std::pow(x / lambda, k-1) * std::exp(-std::pow(x / lambda, k));
    }

    long double weibull_distribution::membership_function(long double x) const {
        if(x < 1e-10L) {
            return 0;
        }
        if (k > 1) {
            long double x_max = lambda * std::pow((k-1) / k, 1/k);
            return std::pow(x / x_max, k-1) * std::exp(std::pow(x_max / lambda, k) - std::pow(x / lambda, k));
        } else {
            long double F = 1 - std::exp(-std::pow(x / lambda, k));
            return 4 * F * (1-F);
        }
    }

    long double weibull_distribution::get_max_x() const {
        if(k > 1) {
            return lambda * std::pow((k-1) / k, 1/k);
        }
        return lambda * std::pow(std::log(2.0L), 1.0L / k);
    }

    long double weibull_distribution::sample(std::mt19937 &generator) const {
        std::uniform_real_distribution<long double> uniform(0.0L, 1.0L);
        const long double u = std::max(uniform(generator), 1e-12L);
        return lambda * std::pow(-std::log(1.0L - u), 1.0L / k);
    }

    long double dichotomy_method(long double a, long double b, std::function<long double(long double)> f, int max_steps, long double eps) {
        if (f(a) * f(b) > 0) {
            return -1;
        }
        int step = 0;
        while ((std::abs(b - a) > eps) && (step < max_steps)) {
            long double c = (a + b) / 2;
            if (f(c) * f(a) < 0) {
                b = c;
            } else {
                a = c;
            }
            step++;
        }
        return (a + b) / 2;

    }

    weibull_distribution generate_random_weibull_distribution() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> random_k(0.1, 5);
        std::uniform_real_distribution<> random_lambda(0.1, 10);
        long double k = random_k(gen);
        long double lambda = random_lambda(gen);
        return weibull_distribution(k, lambda);
    }
}