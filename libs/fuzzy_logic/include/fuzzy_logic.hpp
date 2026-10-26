#pragma once

#include <vector>
#include <functional>
#include <iomanip>
#include <fstream>


#include "weibull_distribution.hpp"


namespace fuzzy_logic {
    class fuzzy_value {
    private:
        std::vector<std::pair<long double, long double>> intervals;
        std::vector<long double> alphas;
    public:
        fuzzy_value(const std::function<long double(long double)> &membership_function, const std::vector<long double>& alphas, long double x_max);    
        fuzzy_value(const fuzzy_value & value);
        std::vector<std::pair<long double, long double>> get_alphas() const;
        long double calc_membership(long double x);

        fuzzy_value& operator=(const fuzzy_value & other);
        fuzzy_value& operator+=(const fuzzy_value & other);
        fuzzy_value& operator-=(const fuzzy_value & other);
        friend std::ostream& operator<<(std::ostream & out, const fuzzy_value & value);
        void save(const std::string &filename) const;
    }; 
    fuzzy_value operator+(const fuzzy_value & first, const fuzzy_value & second);
    fuzzy_value operator-(const fuzzy_value & first, const fuzzy_value & second);
}