# add examples
cmake_minimum_required(VERSION 3.2)
project(astra-samples)

if (ANDROID)
  set(ASTRA_ANDROID TRUE)
elseif (WIN32)
  set(ASTRA_WINDOWS TRUE)
elseif (APPLE)
  set(ASTRA_OSX TRUE)
elseif (UNIX) #not android
  set(ASTRA_UNIX TRUE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
find_package(Astra REQUIRED)

if (NOT ASTRA_SDK_BUILD)
  #copy Astra SDK binaries to lib dir
  if (ASTRA_OSX)
    set(ASTRA_LIB_COPY_TARGET ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
    add_custom_target(copy-astra-libs ALL
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${ASTRA_LIB_DIR}/
      ${ASTRA_LIB_COPY_TARGET})
    set_target_properties(copy-astra-libs PROPERTIES FOLDER CMakeCopyTargets)
  endif()
endif()

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

if (ASTRA_OSX)
  install(
    FILES scripts/build_samples.sh
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
    DESTINATION samples/)
endif()

include_directories(${ASTRA_INCLUDE_DIR})

get_filename_component(SAMPLE_COMMON_DIR "./common" ABSOLUTE)

include_directories(${SAMPLE_COMMON_DIR})

set(ASTRA_CXX_FLAGS "-Wall -fPIC")
set(ASTRA_CXX_FLAGS "${ASTRA_CXX_FLAGS} -std=c++14")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ASTRA_CXX_FLAGS}")

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
#set the rpath to go up from the bin/ to to the lib/ dir to find the Astra libs
if(ASTRA_OSX)
  set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
  message("Setting rpath to ${CMAKE_INSTALL_RPATH}")
elseif(ASTRA_UNIX)
  set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
  message("Setting rpath to ${CMAKE_INSTALL_RPATH}")
endif()

add_subdirectory(c-api)
add_subdirectory(cpp-api)

if(NOT ASTRA_ANDROID)
  add_subdirectory(sfml)
  add_subdirectory(tools)
  if (ASTRA_SKELETON)
      add_dependencies(SimpleSkeletonViewer-SFML orbbec_skeleton)
  endif()
endif()
