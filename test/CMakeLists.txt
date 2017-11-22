add_executable(redux-test ${CMAKE_CURRENT_SOURCE_DIR}/redux.cpp)
target_link_libraries(redux-test redux doctest)
target_compile_features(redux-test PRIVATE cxx_std_17)
add_test(redux-test redux-test)
