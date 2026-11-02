#pragma once

#include <vector>
#include <functional>
#include <iomanip>
#include <fstream>
#include <algorithm>


#include "weibull_distribution.hpp"


namespace fuzzy_logic {
    class fuzzy_value {
    private:
        std::vector<std::pair<long double, long double>> intervals;
        std::vector<long double> alphas;
    public:
        fuzzy_value() = default;
        fuzzy_value(const std::function<long double(long double)> &membership_function, const std::vector<long double>& alphas, long double x_max);    
        fuzzy_value(const fuzzy_value & value);
        std::vector<std::pair<long double, long double>> get_alphas_intervals() const;
        std::vector<long double> get_alphas() const;
        long double calc_membership(long double x) const;

        fuzzy_value& operator=(const fuzzy_value & other);
        fuzzy_value& operator=(fuzzy_value && other);
        fuzzy_value& operator+=(const fuzzy_value & other);
        fuzzy_value& operator-=(const fuzzy_value & other);
        fuzzy_value& operator*=(const fuzzy_value & other);
        fuzzy_value& operator/=(const fuzzy_value & other);
        friend std::ostream& operator<<(std::ostream & out, const fuzzy_value & value);
        void save(const std::string &filename) const;
    }; 
    fuzzy_value operator+(const fuzzy_value & first, const fuzzy_value & second);
    fuzzy_value operator-(const fuzzy_value & first, const fuzzy_value & second);
    fuzzy_value operator*(const fuzzy_value & first, const fuzzy_value & second);
    fuzzy_value operator/(const fuzzy_value & first, const fuzzy_value & second);

    fuzzy_value generate_random_fuzzy_value(const std::vector<long double> &alphas={0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9});
}