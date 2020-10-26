set(_projname "test-harness")

set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
message("Setting rpath to ${CMAKE_INSTALL_RPATH}")

function(add_astra_sdk_dependency _targetname)
  if(NOT TARGET ${_targetname})
    message("add_sdk_dependency: There is no target named '${_targetname}'")
    return()
  endif()
  if(TARGET ${SDK_DEPENDENT_TARGET})
    add_dependencies(${_targetname} ${SDK_DEPENDENT_TARGET})
  endif()
endfunction()

include_directories(${CATCH_INCLUDE_DIR} ${ASTRA_INCLUDE_DIR})

set(${_projname}_SOURCES
  main.cpp
  stream-api/basic_tests.cpp
  stream-api/stream_tests.cpp
#  c-api/DepthReaderPoll_tests.cpp
  )

add_executable(${_projname} ${${_projname}_SOURCES})

target_link_libraries(${_projname} ${ASTRA_LIBRARIES})

add_astra_sdk_dependency(${_projname})

set_target_properties(${_projname} PROPERTIES FOLDER "tests")
set_target_properties(${_projname} PROPERTIES OUTPUT_NAME "astra-tests")
