#pragma once

namespace fuzzy_logic {
    class fuzzy_value {
    private:
        std::function<double(double)>membership_function;
    public:
        fuzzy_value(std::function<double(double)> membership_function);
        
    }
}