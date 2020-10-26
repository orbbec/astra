#SFML path varies depending upon if we're building the full SDK or just sample directory
if (ASTRA_SDK_BUILD)
  if (ASTRA_OSX)
    set(SFML_PATH "${CMAKE_SOURCE_DIR}/samples-aux/thirdparty-bin/osx/sfml-2.3.2-min/")
  else()
      set(SFML_PATH "SFML_ROOT-UNDEFINED")
  endif()
else()
  if (ASTRA_OSX)
      set(SFML_PATH "${CMAKE_SOURCE_DIR}/thirdparty/sfml-2.3.2-min/")
  endif()
endif()

if (DEFINED SFML_PATH)
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${SFML_PATH}")
endif()

add_subdirectory(SimpleDepthViewer-SFML)
add_subdirectory(SimpleHandViewer-SFML)
add_subdirectory(SimpleStreamViewer-SFML)
add_subdirectory(SimpleColorViewer-SFML)
add_subdirectory(MultiSensorViewer-SFML)

if(ASTRA_SKELETON)
    add_subdirectory(SimpleSkeletonViewer-SFML)
endif()

set(FONT_TARGET_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/")

if (ASTRA_SDK_BUILD)
  install_file(${PROJECT_SOURCE_DIR}/vendor/Inconsolata.otf bin)
endif()

if (ASTRA_OSX)
  add_custom_target(copy-sample-files ALL
    COMMAND ${CMAKE_COMMAND} -E copy
    "${PROJECT_SOURCE_DIR}/vendor/Inconsolata.otf"
    ${FONT_TARGET_DIR})

  set_target_properties(copy-sample-files PROPERTIES FOLDER CMakeCopyTargets)

  add_custom_command(TARGET copy-sample-files POST_BUILD
    COMMAND cp
    -R
    ${SFML_PATH}/lib/
    ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/)

  if (ASTRA_SDK_BUILD)
    install_dir(${SFML_PATH}/lib/ lib)
    install_dir(${SFML_PATH} samples/thirdparty/sfml-2.3.2-min)
  endif()
endif()
