#pragma once

#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iomanip>
#include <limits>
#include <numeric>
#include <ostream>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "weibull_distribution.hpp"

namespace fuzzy_logic {

class fuzzy_value {
private:
    std::pair<long double, long double> interval{0.0L, 0.0L};
public:
    fuzzy_value() = default;
    fuzzy_value(long double left, long double right);
    fuzzy_value(const std::function<long double(long double)> &membership_function,
                long double x_max);

    long double left() const noexcept { return interval.first; }
    long double right() const noexcept { return interval.second; }
    std::pair<long double, long double> get_interval() const noexcept { return interval; }
    long double calc_membership(long double x) const noexcept;

    fuzzy_value &operator+=(const fuzzy_value &other);
    fuzzy_value &operator-=(const fuzzy_value &other);
    fuzzy_value &operator*=(const fuzzy_value &other);
    fuzzy_value &operator/=(const fuzzy_value &other);

    void save(const std::string &filename) const;
    friend std::ostream &operator<<(std::ostream &out, const fuzzy_value &value);
};

fuzzy_value operator+(const fuzzy_value &first, const fuzzy_value &second);
fuzzy_value operator-(const fuzzy_value &first, const fuzzy_value &second);
fuzzy_value operator*(const fuzzy_value &first, const fuzzy_value &second);
fuzzy_value operator/(const fuzzy_value &first, const fuzzy_value &second);

int compare(const fuzzy_value &first, const fuzzy_value &second) noexcept;
long double Hausdorff_distance(const std::pair<long double, long double> &first,
                               const std::pair<long double, long double> &second);
long double distanse(const fuzzy_value &first, const fuzzy_value &second);
std::vector<std::vector<long double>> cals_distance_matrix(
    const std::vector<fuzzy_value> &data);

fuzzy_value generate_random_fuzzy_value(long double k, long double lambda);
long double generated_membership(long double x, long double k, long double lambda);

struct k_mediods_res {
    std::vector<int> labels;
    std::vector<int> medoids;
    long double cost{0.0L};
};

k_mediods_res k_medoids(const std::vector<std::vector<long double>> &dist,
                         size_t k, size_t max_iter = 100, int seed = 0);
} // namespace fuzzy_logic
