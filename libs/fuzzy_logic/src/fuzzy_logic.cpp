#include "../include/fuzzy_logic.hpp"

namespace fuzzy_logic {
    fuzzy_value::fuzzy_value(std::function<double(double)> membership_function) : membership_function{membership_function} {

    }
}
