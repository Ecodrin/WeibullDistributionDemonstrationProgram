#include "../include/fuzzy_logic.hpp"

#include <stdexcept>

namespace fuzzy_logic {

fuzzy_value::fuzzy_value(long double left, long double right) {
    if (!std::isfinite(left) || !std::isfinite(right) || left > right) {
        throw std::invalid_argument("границы отрезка должны быть конечными и left <= right");
    }
    interval = {left, right};
}

fuzzy_value::fuzzy_value(const std::function<long double(long double)> &membership_function,
                         long double x_max) {
    if (!membership_function || !std::isfinite(x_max) || x_max < 0.0L) {
        throw std::invalid_argument("некорректные параметры функции принадлежности");
    }
    long double left = 0.0L;
    long double right = std::max(x_max, 1.0L);
    while (membership_function(right) > 1e-4L && right < 1e6L) {
        right *= 2.0L;
    }
    interval = {left, right};
}

long double fuzzy_value::calc_membership(long double x) const noexcept {
    return x >= interval.first && x <= interval.second ? 1.0L : 0.0L;
}

fuzzy_value &fuzzy_value::operator+=(const fuzzy_value &other) {
    interval.first += other.interval.first;
    interval.second += other.interval.second;
    return *this;
}

fuzzy_value &fuzzy_value::operator-=(const fuzzy_value &other) {
    const auto old = interval;
    interval = {old.first - other.interval.second, old.second - other.interval.first};
    return *this;
}

fuzzy_value &fuzzy_value::operator*=(const fuzzy_value &other) {
    const std::vector<long double> values{
        interval.first * other.interval.first, interval.first * other.interval.second,
        interval.second * other.interval.first, interval.second * other.interval.second};
    interval = {*std::min_element(values.begin(), values.end()),
                *std::max_element(values.begin(), values.end())};
    return *this;
}

fuzzy_value &fuzzy_value::operator/=(const fuzzy_value &other) {
    if (other.interval.first <= 0.0L && other.interval.second >= 0.0L) {
        throw std::domain_error("деление на отрезок, содержащий ноль");
    }
    const std::vector<long double> values{
        interval.first / other.interval.first, interval.first / other.interval.second,
        interval.second / other.interval.first, interval.second / other.interval.second};
    interval = {*std::min_element(values.begin(), values.end()),
                *std::max_element(values.begin(), values.end())};
    return *this;
}

void fuzzy_value::save(const std::string &filename) const {
    std::ofstream out(filename);
    if (!out) {
        throw std::runtime_error("не удалось открыть файл журнала");
    }
    out << *this;
}

std::ostream &operator<<(std::ostream &out, const fuzzy_value &value) {
    out << "[" << std::setprecision(10) << value.interval.first << "; "
        << value.interval.second << "]";
    return out;
}

fuzzy_value operator+(const fuzzy_value &first, const fuzzy_value &second) {
    fuzzy_value result(first);
    return result += second;
}
fuzzy_value operator-(const fuzzy_value &first, const fuzzy_value &second) {
    fuzzy_value result(first);
    return result -= second;
}
fuzzy_value operator*(const fuzzy_value &first, const fuzzy_value &second) {
    fuzzy_value result(first);
    return result *= second;
}
fuzzy_value operator/(const fuzzy_value &first, const fuzzy_value &second) {
    fuzzy_value result(first);
    return result /= second;
}

int compare(const fuzzy_value &first, const fuzzy_value &second) noexcept {
    if (first.right() < second.left()) return -1;
    if (first.left() > second.right()) return 1;
    return 0;
}

long double Hausdorff_distance(const std::pair<long double, long double> &first,
                               const std::pair<long double, long double> &second) {
    return std::max(std::abs(first.first - second.first),
                    std::abs(first.second - second.second));
}

long double distanse(const fuzzy_value &first, const fuzzy_value &second) {
    return Hausdorff_distance(first.get_interval(), second.get_interval());
}

std::vector<std::vector<long double>> cals_distance_matrix(
    const std::vector<fuzzy_value> &data) {
    std::vector<std::vector<long double>> result(data.size(),
                                                  std::vector<long double>(data.size()));
    for (size_t i = 0; i < data.size(); ++i)
        for (size_t j = i + 1; j < data.size(); ++j)
            result[i][j] = result[j][i] = distanse(data[i], data[j]);
    return result;
}


long double generated_membership(long double x, long double k, long double lambda) {
    weibull_distribution_math::weibull_distribution w(k, lambda);
    return w.membership_function(x);
}

fuzzy_value generate_random_fuzzy_value(long double k, long double lambda) {
    static std::mt19937 generator(std::random_device{}());
    const auto &distribution = weibull_distribution_math::weibull_distribution(k, lambda);
    const long double upper_bound = std::max(1.0L, distribution.get_max_x() * 3.0L);
    long double center = distribution.sample(generator);
    while (center > upper_bound) {
        center = distribution.sample(generator);
    }
    std::uniform_real_distribution<long double> width_factor(0.03L, 0.18L);
    const long double half_width = std::max(
        0.05L, distribution.get_max_x() * width_factor(generator));
    return fuzzy_value(std::max(0.0L, center - half_width), center + half_width);
}

k_mediods_res k_medoids(const std::vector<std::vector<long double>> &dist, size_t k,
                         size_t max_iter, int seed) {
    const size_t n = dist.size();
    if (n == 0 || k == 0 || k > n) throw std::invalid_argument("некорректное число кластеров");
    std::mt19937 gen(seed);
    std::vector<int> medoids;
    medoids.reserve(k);
    std::uniform_int_distribution<size_t> first_distribution(0, n - 1);
    medoids.push_back(static_cast<int>(first_distribution(gen)));
    while (medoids.size() < k) {
        int next = -1;
        long double greatest_distance = -1.0L;
        for (size_t candidate = 0; candidate < n; ++candidate) {
            if (std::find(medoids.begin(), medoids.end(),
                          static_cast<int>(candidate)) != medoids.end()) continue;
            long double nearest = std::numeric_limits<long double>::max();
            for (const int medoid : medoids) {
                nearest = std::min(nearest, dist[candidate][medoid]);
            }
            if (nearest > greatest_distance) {
                greatest_distance = nearest;
                next = static_cast<int>(candidate);
            }
        }
        medoids.push_back(next);
    }
    std::vector<int> labels(n, -1);

    for (size_t iteration = 0; iteration < max_iter; ++iteration) {
        bool changed = false;
        for (size_t i = 0; i < n; ++i) {
            int best = 0;
            for (size_t c = 1; c < k; ++c)
                if (dist[i][medoids[c]] < dist[i][medoids[best]]) best = static_cast<int>(c);
            if (labels[i] != best) { labels[i] = best; changed = true; }
        }
        for (size_t c = 0; c < k; ++c) {
            int best = medoids[c];
            long double best_cost = std::numeric_limits<long double>::max();
            for (size_t candidate = 0; candidate < n; ++candidate) {
                if (labels[candidate] != static_cast<int>(c)) continue;
                long double cost = 0.0L;
                for (size_t other = 0; other < n; ++other)
                    if (labels[other] == static_cast<int>(c)) cost += dist[candidate][other];
                if (cost < best_cost) { best_cost = cost; best = static_cast<int>(candidate); }
            }
            if (best_cost == std::numeric_limits<long double>::max()) {
                int farthest = 0;
                long double farthest_distance = -1.0L;
                for (size_t candidate = 0; candidate < n; ++candidate) {
                    if (std::find(medoids.begin(), medoids.end(),
                                  static_cast<int>(candidate)) != medoids.end()) continue;
                    long double nearest = std::numeric_limits<long double>::max();
                    for (const int medoid : medoids) {
                        nearest = std::min(nearest, dist[candidate][medoid]);
                    }
                    if (nearest > farthest_distance) {
                        farthest_distance = nearest;
                        farthest = static_cast<int>(candidate);
                    }
                }
                best = farthest;
            }
            if (medoids[c] != best) { medoids[c] = best; changed = true; }
        }
        if (!changed) break;
    }
    long double cost = 0.0L;
    for (size_t i = 0; i < n; ++i) cost += dist[i][medoids[labels[i]]];
    return {labels, medoids, cost};
}
} // namespace fuzzy_logic
