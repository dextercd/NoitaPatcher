add_library(nsew_common
    rectangle_optimiser.cpp
)

target_include_directories(nsew_common
    PUBLIC include
)

target_link_libraries(nsew_common
    PUBLIC
        # absl::flat_hash_set
        absl::btree
)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
