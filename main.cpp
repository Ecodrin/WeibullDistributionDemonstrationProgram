#include <iostream>
#include <thread>
#include <chrono>

#include "weibull_distribution.hpp"
#include "fuzzy_logic.hpp"


int main() {
    weibull_distribution_math::weibull_distribution w(5, 1);
    std::vector <long double> alphas = {0.1, 0.2, 0.5, 0.6, 0.9};
    fuzzy_logic::fuzzy_value v([&w](long double x) {
        return w.membership_function(x);
    }, alphas, w.get_max_x());
    // auto res = v.get_alphas();
    // for(const auto & e: res ) {
    //     std::cout << e.first << " " << e.second << std::endl;
    // }
    // std::cout << v.calc_membership(1) << std::endl;
    std::cout << v;
    weibull_distribution_math::weibull_distribution w2(6, 2);
    std::vector <long double> alphas2 = {0.1, 0.2, 0.5, 0.6, 0.9};
    fuzzy_logic::fuzzy_value v2([&w2](long double x) {
        return w2.membership_function(x);
    }, alphas2, w2.get_max_x());
    std::cout << v2;
    fuzzy_logic::fuzzy_value res1 = (v2 + v);
    std::cout << res1;
    fuzzy_logic::fuzzy_value res2 = (v2 - v);
    std::cout << res2;
    std::cout << v.calc_membership(1) << " " << v2.calc_membership(1) << " " << res1.calc_membership(1) <<  " " << res2.calc_membership(1) << std::endl;


    v.save("1.log");
}