set (_projname "astra-core-tests")

set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
message("Setting rpath to ${CMAKE_INSTALL_RPATH}")

set(${_projname}_TESTS
  signal_tests.cpp)

add_executable(${_projname} ${${_projname}_TESTS})

set_target_properties(${_projname} PROPERTIES FOLDER "tests")

target_link_libraries(${_projname} ${ASTRA_LIBRARIES})


