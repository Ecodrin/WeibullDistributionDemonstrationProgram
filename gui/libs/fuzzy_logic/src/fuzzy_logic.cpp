#include "../include/fuzzy_logic.hpp"

namespace fuzzy_logic {
    fuzzy_value::fuzzy_value(const std::function<long double(long double)> & membership_function, const std::vector <long double> & alphas, long double x_max) :alphas{alphas} {
        for(int i = 0; i < alphas.size();++i) {
            long double step = 1.0, left = 0;
            while(membership_function(left) > alphas[i]) {
                step *= 2.0L;
                left = x_max - step;
            }
            long double L = weibull_distribution_math::dichotomy_method(left, x_max, [&](long double x) {
                return membership_function(x) - alphas[i];
            });
            long double right = 2 * x_max;
            while(membership_function(right) > alphas[i]) {
                right *= 2.0;
            }
            long double R = weibull_distribution_math::dichotomy_method(x_max, right, [&](long double x) {
                return membership_function(x) - alphas[i];
            });
            intervals.push_back({L, R});
        }
        this->alphas.push_back(1);
        intervals.push_back({x_max, x_max});
    }

    fuzzy_value::fuzzy_value(const fuzzy_value &value) : alphas{value.alphas}, intervals{value.intervals} {}

    std::vector<std::pair<long double, long double>> fuzzy_value::get_alphas_intervals() const {
        return intervals;
    }

    std::vector<long double> fuzzy_value::get_alphas() const {
        return alphas;
    }

    long double fuzzy_value::calc_membership(long double x) const {
        long double top = intervals[intervals.size()-1].first;
        if (x < top) {
            if (x < intervals[0].first) {
                if (intervals[0].first < 1e-10) {
                    return 0;
                }
                long double v = x / intervals[0].first * alphas[0];
                if (v < 1e-10) {
                    return 0;
                }
                return v;
            }
            for(int i = 1; i < intervals.size() ; ++i) {
                if (x <= intervals[i].first) {
                    long double dx = intervals[i].first - intervals[i-1].first;
                    if (dx < 1e-10L) {
                        return alphas[i];
                    }
                    return alphas[i-1] + (x - intervals[i-1].first) / dx * (alphas[i] - alphas[i-1]);
                }
            }
        } else {
            if (x > intervals[0].second) {
                long double dx = intervals[0].second - intervals[1].second;
                if(dx < 1e-10L) {
                    return 0.0;
                }
                long double slope = (alphas[0]-alphas[1]) / dx;
                long double v = alphas[0] + slope * (x - intervals[0].second);
                // if (v < 1e-10) {
                //     return 0;
                // }
                return v;
            }
            for(int i = intervals.size()-1; i > 0 ; --i) {
                if (x >= intervals[i].second && x <= intervals[i-1].second) {
                    long double dx = intervals[i-1].second - intervals[i].second;
                    if (dx < 1e-10L) {
                        return alphas[i];
                    }
                    return alphas[i] + (x - intervals[i].second) / dx * (alphas[i-1] - alphas[i]);
                }
            }
        }
        return 0;
    }
    fuzzy_value &fuzzy_value::operator=(const fuzzy_value &other) {
        if (this != &other) {
            intervals = other.intervals;
            alphas = other.alphas;
        }
        return *this;
    }


    fuzzy_value& fuzzy_value::operator=(fuzzy_value && other) {
        if (this != &other) {
            intervals = other.intervals;
            alphas = other.alphas;
            other.intervals = {};
        }
        return *this;
    }

    fuzzy_value &fuzzy_value::operator+=(const fuzzy_value &other) {
        if (alphas != other.alphas) {
            throw std::runtime_error("incorrect alphas");
        }
        for(size_t i = 0; i < intervals.size();++i) {
            intervals[i].first += other.intervals[i].first;
            intervals[i].second += other.intervals[i].second;
        }
        return *this;
    }
    fuzzy_value &fuzzy_value::operator-=(const fuzzy_value &other) {
        if (alphas != other.alphas) {
            throw std::runtime_error("incorrect alphas");
        }
        for(size_t i = 0; i < intervals.size();++i) {
            intervals[i].first -= other.intervals[i].second;
            intervals[i].second -= other.intervals[i].first;
        }
        return *this;
    }
    fuzzy_value &fuzzy_value::operator*=(const fuzzy_value &other) {
        if (alphas != other.alphas) {
            throw std::runtime_error("incorrect alphas");
        }
        for(size_t i = 0; i < intervals.size(); ++i) {
            std::vector<long double> values = {intervals[i].first * other.intervals[i].first, intervals[i].first * other.intervals[i].second, 
                                        intervals[i].second * other.intervals[i].first, intervals[i].second * other.intervals[i].second};
            intervals[i].first = *std::min_element(values.begin(), values.end());
            intervals[i].second = *std::max_element(values.begin(), values.end());
        }
        return *this;
    }

    fuzzy_value &fuzzy_value::operator/=(const fuzzy_value &other) {
        if (alphas != other.alphas) {
            throw std::runtime_error("incorrect alphas");
        }
        for(size_t i = 0; i < intervals.size(); ++i) {
            std::vector<long double> values = {intervals[i].first / other.intervals[i].first, intervals[i].first / other.intervals[i].second, 
                                        intervals[i].second / other.intervals[i].first, intervals[i].second / other.intervals[i].second};
            intervals[i].first = *std::min_element(values.begin(), values.end());
            intervals[i].second = *std::max_element(values.begin(), values.end());
        }
        return *this;
    }

    void fuzzy_value::save(const std::string &filename) const
    {
        std::ofstream out;
        out.open(filename);
        out << (*this);
        out.close();
    }
    std::ostream &operator<<(std::ostream &out, const fuzzy_value &value)
    {
        out << "================================================\n";
        for(size_t i = 0; i < value.alphas.size(); ++i) {
            out << std::setw(10) << std::setfill(' ') << value.alphas[i] 
                    << std::setw(16) << std::setfill(' ') << value.intervals[i].first 
                    << std::setw(16) << std::setfill(' ') << value.intervals[i].second << std::endl;
        }
        out << "================================================\n" << std::endl;
        return out;
    }
    fuzzy_value operator+(const fuzzy_value &first, const fuzzy_value &second) {
        fuzzy_value copy(first);
        return copy += second;
    }
    fuzzy_value operator-(const fuzzy_value &first, const fuzzy_value &second) {
        fuzzy_value copy(first);
        return copy -= second;
    }
    fuzzy_value operator*(const fuzzy_value &first, const fuzzy_value &second) {
        fuzzy_value copy(first);
        return copy *= second;
    }
    fuzzy_value operator/(const fuzzy_value &first, const fuzzy_value &second) {
        fuzzy_value copy(first);
        return copy /= second;
    }

    fuzzy_value generate_random_fuzzy_value(const std::vector<long double> &alphas) {
        weibull_distribution_math::weibull_distribution w = weibull_distribution_math::generate_random_weibull_distribution();
        return fuzzy_value([&](long double x){
            return w.membership_function(x);
        }, alphas, w.get_max_x());
    }


    long double Hausdorff_distance(const std::pair<long double, long double> & first, const std::pair<long double, long double> & second) {
        return std::max(std::abs(first.first - second.first), std::abs(first.second - second.second));
    }

    long double distanse(const fuzzy_value & first, const fuzzy_value & second, const std::vector<long double> & w) {
        if (first.get_alphas() != second.get_alphas()) {
            throw std::runtime_error("альфа уровни должны быть одинаковые");
        }
        long double sum = 0;
        for(size_t i = 0; i < first.get_alphas().size(); ++i) {
            sum += w[i] * Hausdorff_distance(first.get_alphas_intervals()[i], second.get_alphas_intervals()[i]);
        }
        return sum;
    }

    std::vector<std::vector<long double>> cals_distance_matrix(const std::vector<fuzzy_value> & data, const std::vector<long double> & w) {
        size_t n = data.size();
        std::vector<std::vector<long double>> distance_matrix(n, std::vector<long double>(n, 0.0));
        for(size_t i = 0; i < n; ++i) {
            for(size_t j = i+1; j < n; ++j) {
                long double dist = distanse(data[i], data[j], w);
                distance_matrix[i][j] = dist;
                distance_matrix[j][i] = dist;
            }
        }
        return distance_matrix;
    }

    std::vector<long double> calculate_weights(const std::vector<long double> alphas) {
        std::vector<long double> w(alphas.size(), 0.0);
        for(size_t i = 0; i < alphas.size(); ++i) {
            long double left = (i == 0) ? 0.0 : (alphas[i] - alphas[i-1]);
            long double right = (i == alphas.size()-1) ? 0.0 : (alphas[i+1] - alphas[i]);
            w[i] = 0.5 * (left + right);
        }
        long double sum = std::accumulate(w.begin(), w.end(), 0.0);
        for(size_t i = 0; i < alphas.size(); ++i) {
            w[i] /= sum;
        }
        return w;
    }

    k_mediods_res k_medoids(const std::vector<std::vector<long double>> &dist, size_t k, size_t max_iter, int seed) {
        size_t n = dist.size();
        std::mt19937 gen(seed);
        std::vector<int> idx(n);
        std::iota(idx.begin(), idx.end(), 0);
        std::shuffle(idx.begin(), idx.end(), gen);
        std::vector<int> medoids(idx.begin(), idx.begin() + k);
        std::vector<int> labels(n, -1);
        std::vector<long double> best_dist(n, 0.0);

        for(size_t iter = 0; iter < max_iter; ++iter) {
            bool changed = false;
            for(size_t i = 0; i < n; ++i) {
                int best_c = 0;
                long double best_d = dist[i][medoids[0]];
                for(int c = 1; c < k; ++c) {
                    long double d = dist[i][medoids[c]];
                    if(d < best_d) {
                        best_c = c;
                        best_d = d;
                    }
                }
                if (labels[i] != best_c) {
                    labels[i] = best_c;
                    changed = true;
                }
                best_dist[i] = best_d;
            }

            for(int c = 0; c < k; ++c) {
                std::vector<int> members;
                for(size_t i = 0; i < n;++i) {
                    if(labels[i] == c) {
                        members.push_back(i);
                    }
                }
                if(members.empty()) {
                    continue;
                }

                int best_medoid = medoids[c];
                long double best_cost = std::numeric_limits<long double>::max();
                for(int cand : members) {
                    long double cost = 0.0;
                    for(int other : members) {
                        cost += dist[cand][other];
                    }
                    if (cost < best_cost) {
                        best_cost = cost;
                        best_medoid = cand;
                    }
                }

                if(best_medoid != medoids[c]) {
                    medoids[c] = best_medoid;
                    changed = true;
                }
            }
            if(!changed) {
                break;
            }
        }
        long double cost = 0.0;
        for(size_t i = 0; i < n; ++i) {
            cost += dist[i][medoids[labels[i]]];
        }
        return {labels, medoids, cost};
    }

}
