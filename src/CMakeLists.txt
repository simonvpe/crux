add_library(redux INTERFACE)

target_include_directories(redux INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/redux>)

target_compile_definitions(redux
    INTERFACE LIBRARY_HEADER_ONLY
)
 
#set_target_properties(pid
#  PROPERTIES PUBLIC_HEADER ${CMAKE_SOURCE_DIR}/include/pid.hpp)

#target_compile_features(pid
#  PRIVATE cxx_std_17)

install(TARGETS redux
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include/redux)
