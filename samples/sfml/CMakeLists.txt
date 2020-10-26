#SFML path varies depending upon if we're building the full SDK or just sample directory
if (ASTRA_SDK_BUILD)
    set(SFML_PATH "SFML_ROOT-UNDEFINED")
endif()

if (DEFINED SFML_PATH)
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${SFML_PATH}")
endif()

add_subdirectory(SimpleDepthViewer-SFML)
add_subdirectory(SimpleHandViewer-SFML)
add_subdirectory(SimpleStreamViewer-SFML)
add_subdirectory(SimpleColorViewer-SFML)
add_subdirectory(MultiSensorViewer-SFML)

set(FONT_TARGET_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/")

if (ASTRA_SDK_BUILD)
  install_file(${PROJECT_SOURCE_DIR}/vendor/Inconsolata.otf bin)
endif()
