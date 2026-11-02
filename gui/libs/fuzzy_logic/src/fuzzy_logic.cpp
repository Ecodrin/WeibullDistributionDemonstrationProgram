#include "../include/fuzzy_logic.hpp"

namespace fuzzy_logic {
    fuzzy_value::fuzzy_value(const std::function<long double(long double)> & membership_function, const std::vector <long double> & alphas, long double x_max) :alphas{alphas} {
        this->alphas.push_back(0.999);
        for(int i = 0; i < alphas.size();++i) {
            long double L = weibull_distribution_math::dichotomy_method(0, x_max, [&](long double x) {
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
        long double L = weibull_distribution_math::dichotomy_method(0, x_max, [&](long double x) {
            return membership_function(x) - 0.9999;
        });
        long double right = 2 * x_max;
        while(membership_function(right) > 0.9999) {
            right *= 2.0;
        }
        long double R = weibull_distribution_math::dichotomy_method(x_max, right, [&](long double x) {
            return membership_function(x) - 0.9999;
        });
        intervals.push_back({L, R});
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
                return x / intervals[0].first * alphas[0];
            }
            for(int i = 1; i < intervals.size() ; ++i) {
                if (x >= intervals[i-1].first && x <= intervals[i].first) {
                    return alphas[i-1] + (x - intervals[i-1].first) / (intervals[i].first - intervals[i-1].first) * (alphas[i] - alphas[i-1]);
                }
            }
        } else {
            if (x > intervals[0].second) {
                return x / intervals[0].second * alphas[0];
            }
            for(int i = intervals.size()-1; i > 0 ; --i) {
                if (x >= intervals[i].second && x <= intervals[i-1].second) {
                    return alphas[i] + (x - intervals[i].second) / (intervals[i-1].second - intervals[i].second) * (alphas[i-1] - alphas[i]);
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
}
