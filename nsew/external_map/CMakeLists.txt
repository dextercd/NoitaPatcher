find_package(SFML 2.5.1 REQUIRED graphics network)

add_executable(external_map main.cpp)
target_link_libraries(external_map PRIVATE sfml-graphics sfml-network)
