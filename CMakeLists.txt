cmake_minimum_required (VERSION 3.2)

project (Astra)

if (APPLE)
  set(ASTRA_OSX TRUE)
elseif (UNIX) #not android
  set(ASTRA_UNIX TRUE)
endif()

# Version
string (TIMESTAMP BUILD_DATE "%Y%m%dT%H%M%SZ" UTC)
set (Astra_VERSION_MAJOR 0)
set (Astra_VERSION_MINOR 5)
set (Astra_VERSION_PATCH 0)
set (Astra_VERSION_BUILD 0)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(ASTRA_64 ON)
else()
  set(ASTRA_32 ON)
endif()

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(ASTRA_CXX_FLAGS "-Wall -fPIC -std=c++14")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ASTRA_CXX_FLAGS}")

message("flags ${CMAKE_CXX_FLAGS}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(ASTRA_DOCS FALSE CACHE BOOL "Build documentation")
set(ASTRA_XS TRUE CACHE BOOL "Build extra stream support plugin")
set(ASTRA_HAND TRUE CACHE BOOL "Build hand tracking plugin")
set(ASTRA_STREAMPLAYER FALSE CACHE BOOL "Build experimental stream playback plugin (not working)")
set(ASTRA_MOCK_DEVICE FALSE CACHE BOOL "Build mock test device plugin")
set(ASTRA_SKELETON FALSE CACHE BOOL "Build experimental skeleton support (not working)")

if (ASTRA_EXPERIMENTAL)
  set(ASTRA_STREAMPLAYER TRUE)
  set(ASTRA_MOCK_DEVICE TRUE)
  set(ASTRA_SKELETON TRUE)
endif()

if (ASTRA_DOCS)
  add_subdirectory(docs)
endif()

find_package(CLISP REQUIRED)

#Override FindAstra equivalents
set(ASTRA_SDK_BUILD TRUE)
set(ASTRA_LIBRARIES
  astra_core
  astra_core_api
  astra
  )
set(ASTRA_INCLUDE_DIR
  ${PROJECT_SOURCE_DIR}/include
  )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(SAMPLE_DIR_FOLDER "samples/")

set (SDK_DEPENDENT_TARGET "SDK_dependent")
add_custom_target(${SDK_DEPENDENT_TARGET})

set(SHINY_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/vendor/shinyprofiler/include"
  CACHE
  INTERNAL
  "Path to include folder for Shiny")

set(CATCH_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/vendor/catch/include"
  CACHE
  INTERNAL
  "Path to include folder for Catch")

set(TINYDIR_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/vendor/tinydir"
  CACHE
  INTERNAL
  "Path to include folder for tinydir")

set(CPPTOML_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/vendor/cpptoml"
  CACHE
  INTERNAL
  "Path to include folder for cpptoml")

set(EASYLOGGINGPP_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/vendor/easyloggingpp"
  CACHE
  INTERNAL
  "Path to include folder for easylogging++")

if (ASTRA_64)
  set(SAMPLE_DEPENDENCY_DIR "copy_to_bin64_dir")
else()
  set(SAMPLE_DEPENDENCY_DIR "copy_to_bin32_dir")
endif()
if (MSVC)
  if (MSVC_VERSION EQUAL 1800)
    set(VS_DIR "vs2013")
  elseif(MSVC_VERSION EQUAL 1900)
    set(VS_DIR "vs2015")
  else()
    error("MSVC other than VS2013 (VC12) or VS2015 (VC14) are not supported")
  endif()
endif()

function(install_lib _targetname)
  set(extrapath ${ARGV1})
  install(TARGETS ${_targetname}
    RUNTIME DESTINATION bin/${extrapath}
    LIBRARY DESTINATION lib/${extrapath}
    ARCHIVE DESTINATION lib/${extrapath}
    )
  if (MSVC)
    install_pdb(${_targetname} bin/)
    set(INSTALL_PATHS
      samples/${VS_DIR}/${SAMPLE_DEPENDENCY_DIR}/${extrapath}
      samples/${VS_DIR}/bin/Debug/${extrapath}
      samples/${VS_DIR}/bin/Release/${extrapath}
      )
    foreach(INSTALL_PATH ${INSTALL_PATHS})
      install(TARGETS ${_targetname}
        RUNTIME DESTINATION ${INSTALL_PATH}
        )
      install_pdb(${_targetname} ${INSTALL_PATH})
    endforeach()
  endif()
endfunction()

function(install_file _targetfile basepath)
  set(extrapath ${ARGV2})
  if (MSVC)
    set(INSTALL_PATHS
      bin/${extrapath}
      samples/${VS_DIR}/${SAMPLE_DEPENDENCY_DIR}/${extrapath}
      samples/${VS_DIR}/bin/Debug/${extrapath}
      samples/${VS_DIR}/bin/Release/${extrapath}
      )
    foreach(INSTALL_PATH ${INSTALL_PATHS})
      install(FILES ${_targetfile}
        DESTINATION ${INSTALL_PATH}
        )
    endforeach()
  else()
    install(FILES ${_targetfile}
      DESTINATION ${basepath}/${extrapath}
      )
  endif()
endfunction()

function(install_dir _targetdir basepath)
  set(extrapath ${ARGV2})
  if (MSVC)
    set(INSTALL_PATHS
      bin/${extrapath}
      samples/${VS_DIR}/${SAMPLE_DEPENDENCY_DIR}/${extrapath}
      samples/${VS_DIR}/bin/Debug/${extrapath}
      samples/${VS_DIR}/bin/Release/${extrapath}
      )
    foreach(INSTALL_PATH ${INSTALL_PATHS})
      install(DIRECTORY ${_targetdir}
        DESTINATION ${INSTALL_PATH}
        )
    endforeach()
  else()
    install(DIRECTORY ${_targetdir}
      DESTINATION ${basepath}/${extrapath}
      )
  endif()
endfunction()

set(ASTRA_SDK_BINARY_DIR ${PROJECT_BINARY_DIR})
set(ASTRA_SDK_SOURCE_DIR ${PROJECT_SOURCE_DIR})

add_subdirectory(vendor)
add_subdirectory(src)
add_subdirectory(samples)
add_subdirectory(tests)

set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/sdk")
MESSAGE("CMAKE_INSTALL_PREFIX : ${CMAKE_INSTALL_PREFIX}")

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include
  DESTINATION .
  PATTERN "*.lpp" EXCLUDE
  PATTERN "include/astra/Plugins" EXCLUDE
  PATTERN ".pp-modification-cache.cl" EXCLUDE
  )

install(DIRECTORY ${PROJECT_SOURCE_DIR}/samples
  DESTINATION .
  PATTERN "build" EXCLUDE
  PATTERN ".gitignore" EXCLUDE
  PATTERN "samples/scripts" EXCLUDE
  PATTERN "samples/plugins/orbbec_skeleton" EXCLUDE
  PATTERN "samples/sfml/SimpleSkeletonViewer-SFML" EXCLUDE
  PATTERN "samples/tools/StreamRecorder" EXCLUDE
  )

if (ASTRA_UNIX)
  install(DIRECTORY ${PROJECT_SOURCE_DIR}/install
    DESTINATION .
    )
endif()

install(DIRECTORY ${PROJECT_SOURCE_DIR}/cmake
  DESTINATION samples
  PATTERN "build" EXCLUDE
  PATTERN ".gitignore" EXCLUDE
  )

if (MSVC)
  #copy third party build files to install Visual Studio directory
  #Lack of a trailing / is important here, thirdparty ends up in samples/${VS_DIR}/
  install(DIRECTORY ${PROJECT_SOURCE_DIR}/samples-aux/thirdparty-bin/${VS_DIR}/thirdparty
    DESTINATION samples/${VS_DIR}/
    PATTERN "build" EXCLUDE
    PATTERN ".gitignore" EXCLUDE
    PATTERN ".git" EXCLUDE
    )

  #copy samples runtime files to all the places Visual Studio would want them
  set(INSTALL_PATHS
    bin/
    samples/${VS_DIR}/${SAMPLE_DEPENDENCY_DIR}/
    samples/${VS_DIR}/bin/Debug/
    samples/${VS_DIR}/bin/Release/
    )
  foreach(INSTALL_PATH ${INSTALL_PATHS})
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/samples-aux/thirdparty-bin/${VS_DIR}/${SAMPLE_DEPENDENCY_DIR}/
      DESTINATION ${INSTALL_PATH})
  endforeach()

  #prepare to generate Visual Studio solution for installation
  set(SAMPLES_SOURCE_DIR "${PROJECT_BINARY_DIR}/sdk/samples")
  set(SAMPLES_BUILD_TEMP_DIR "${PROJECT_BINARY_DIR}/temp/${VS_DIR}/")

  #This doesn't replace anything except line endings for Windows CRLF style
  configure_file("${PROJECT_SOURCE_DIR}/samples-aux/README.txt"
    "${PROJECT_BINARY_DIR}/temp/README.txt"
    @ONLY
    NEWLINE_STYLE CRLF)
  configure_file("${PROJECT_SOURCE_DIR}/LICENSE.txt"
    "${PROJECT_BINARY_DIR}/temp/LICENSE.txt"
    @ONLY
    NEWLINE_STYLE CRLF)

  #install the CRLF line ending readme and license
  install(FILES
    ${PROJECT_BINARY_DIR}/temp/README.txt
    ${PROJECT_BINARY_DIR}/temp/LICENSE.txt
    DESTINATION .
    )

  #Configure Visual Studio project generation & cleanup script
  configure_file("${PROJECT_SOURCE_DIR}/samples-aux/InstallVS.cmake.in"
    "${PROJECT_BINARY_DIR}/InstallVS.cmake"
    @ONLY)
  #Run the script at install time to generate VS projects for the samples
  #also de-cmake-ifies the projects and makes everything a relative directory
  install(SCRIPT "${PROJECT_BINARY_DIR}/InstallVS.cmake")

  #install the sample VS projects
  install(DIRECTORY ${SAMPLES_BUILD_TEMP_DIR}
    DESTINATION samples/${VS_DIR}
    PATTERN "build" EXCLUDE
    PATTERN ".gitignore" EXCLUDE
    )
  #remove temporary build dir
  install(CODE "file(REMOVE_RECURSE ${SAMPLES_BUILD_TEMP_DIR})")
else()
  #for non-MSVC platforms, just install the readme and license directly
  install(FILES
    ${PROJECT_SOURCE_DIR}/samples-aux/README.txt
    ${PROJECT_SOURCE_DIR}/LICENSE.txt
    DESTINATION .
    )
endif()

set(CPACK_PACKAGE_NAME "AstraSDK")
set(CPACK_PACKAGE_VENDOR "Orbbec")
set(CPACK_PACKAGE_DIRECTORY "${PROJECT_BINARY_DIR}/sdk_package")
set(CPACK_PACKAGE_VERSION_MAJOR ${Astra_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${Astra_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH "${Astra_VERSION_PATCH}-${BUILD_DATE}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Orbbec Astra SDK")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Orbbec/Astra")

message("CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
string(TOLOWER "${CMAKE_SYSTEM_NAME}" system_name_lower)
if (system_name_lower STREQUAL "darwin")
  if (ASTRA_64)
    set(CMAKE_SYSTEM_NAME "darwin-x64")
  else()
    error("32-bit OS X not supported")
  endif()
  message("Setting CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
elseif(system_name_lower STREQUAL "windows")
  if (ASTRA_64)
    set(CMAKE_SYSTEM_NAME "${VS_DIR}-win64")
  else()
    set(CMAKE_SYSTEM_NAME "${VS_DIR}-win32")
  endif()
  message("Setting CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
endif()

set(CPACK_GENERATOR "TGZ")

INCLUDE(CPack)
