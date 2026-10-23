add_library(fuzzy_logic_library
    include/fuzzy_logic.hpp
    src/fuzzy_logic.cpp
)

target_include_directories(fuzzy_logic_library PUBLIC include)
target_link_libraries(fuzzy_logic_library weibull_distribution_math_library)