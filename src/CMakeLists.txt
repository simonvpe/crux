add_library(state-mgmt INTERFACE)

target_include_directories(state-mgmt INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/state-mgmt>)

target_compile_definitions(state-mgmt
    INTERFACE LIBRARY_HEADER_ONLY
)
 
#set_target_properties(pid
#  PROPERTIES PUBLIC_HEADER ${CMAKE_SOURCE_DIR}/include/pid.hpp)

#target_compile_features(pid
#  PRIVATE cxx_std_17)

install(TARGETS state-mgmt
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include/state-mgmt)
