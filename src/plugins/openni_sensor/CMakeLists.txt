set (_projname "openni_sensor")

set(${_projname}_HEADERS
  oni_adapter_plugin.hpp
  oni_colorstream.hpp
  oni_depthstream.hpp
  oni_device_streamset.hpp
  oni_devicestream.hpp
  oni_infrared_stream.hpp
  oni_mappers.hpp
  oni_stream.hpp
  )

set(${_projname}_SOURCES
  oni_adapter_plugin.cpp
  oni_colorstream.cpp
  oni_depthstream.cpp
  oni_device_streamset.cpp
  oni_infrared_stream.cpp
  oni_mappers.cpp
  )

add_definitions(-DASTRA_BUILD)

find_library(OpenNI2_LIBRARY NAMES "libOpenNI2.so" HINTS "$ENV{OPENNI2_REDIST}" PATH_SUFFIXES lib)

#extend the rpath to include the openni2 subdirectory
set(CMAKE_INSTALL_RPATH "$ORIGIN/..;$ORIGIN/openni2")

include_directories ("${CMAKE_SOURCE_DIR}/vendor/openni2/include")

include_directories(${_projname} ${SHINY_INCLUDE_DIR})

add_library(${_projname} SHARED ${${_projname}_SOURCES} ${${_projname}_HEADERS})

set_target_properties(${_projname} PROPERTIES FOLDER "plugins")

target_link_libraries(${_projname} astra_core_api Shiny ${OpenNI2_LIBRARY})

install_lib(${_projname} "Plugins/")

set(openni2_vendor_dir ${CMAKE_SOURCE_DIR}/vendor/openni2/ubuntu-x64/)
set(openni2_target_dir "${PROJECT_BINARY_DIR}/lib/Plugins/openni2")
set(openni2_target_install_dir "Plugins/openni2")

add_custom_command(TARGET ${_projname} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${openni2_vendor_dir} ${openni2_target_dir})

install_dir(${openni2_vendor_dir} lib ${openni2_target_install_dir})
