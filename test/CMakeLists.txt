add_executable(state-mgmt-test ${CMAKE_CURRENT_SOURCE_DIR}/state-mgmt.cpp)
target_link_libraries(state-mgmt-test state-mgmt doctest)
target_compile_features(state-mgmt-test PRIVATE cxx_std_17)
add_test(state-mgmt-test state-mgmt-test)
