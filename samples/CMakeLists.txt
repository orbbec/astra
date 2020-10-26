# add examples
cmake_minimum_required(VERSION 3.2)
project(astra-samples)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
find_package(Astra REQUIRED)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(ASTRA_64 ON)
else()
  set(ASTRA_32 ON)
endif()

include(ConfigUserFile)

function(add_astra_sdk_dependency _targetname)
  if(NOT TARGET ${_targetname})
    message("add_sdk_dependency: There is no target named '${_targetname}'")
    return()
  endif()
  if(TARGET ${SDK_DEPENDENT_TARGET})
    add_dependencies(${_targetname} ${SDK_DEPENDENT_TARGET})
  endif()
endfunction()

function(install_sample _targetname)
  if (ASTRA_SDK_BUILD)
    install(TARGETS ${_targetname} DESTINATION bin/)
  endif()
endfunction()

include_directories(${ASTRA_INCLUDE_DIR})

get_filename_component(SAMPLE_COMMON_DIR "./common" ABSOLUTE)

include_directories(${SAMPLE_COMMON_DIR})

set(ASTRA_CXX_FLAGS "-Wall -fPIC")
set(ASTRA_CXX_FLAGS "${ASTRA_CXX_FLAGS} -std=c++14")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ASTRA_CXX_FLAGS}")

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
#set the rpath to go up from the bin/ to to the lib/ dir to find the Astra libs
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
message("Setting rpath to ${CMAKE_INSTALL_RPATH}")

add_subdirectory(c-api)
add_subdirectory(cpp-api)

add_subdirectory(sfml)
