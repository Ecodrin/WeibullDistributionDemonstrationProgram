add_library(fuzzy_logic SHARED
    include/fuzzy_logic.hpp
    src/fuzzy_logic.hpp
)

target_include_directories(fuzzy_logic PUBLIC include)
