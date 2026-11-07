#pragma once

#include <vector>
#include <functional>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <random>


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
    long double Hausdorff_distance(const std::pair<long double, long double> & first, const std::pair<long double, long double> & second);
    long double distanse(const fuzzy_value & first, const fuzzy_value & second, const std::vector<long double> & w);\
    std::vector<long double> calculate_weights(const std::vector<long double> alphas);
    std::vector<std::vector<long double>> cals_distance_matrix(const std::vector<fuzzy_value> & data, const std::vector<long double> & w);

    fuzzy_value generate_random_fuzzy_value(const std::vector<long double> &alphas={0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9});


    struct k_mediods_res {
        std::vector<int> labels;
        std::vector<int> medoids;
        long double cost;
    };

    k_mediods_res k_medoids(const std::vector<std::vector<long double>> &dist, size_t k, size_t max_iter = 100, int seed = 0);
}