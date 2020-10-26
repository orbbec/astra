# add subdirs

include_directories(${ASTRA_INCLUDE_DIR})

# First for the generic no-config case (e.g. with mingw)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/Plugins)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/Plugins)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/Plugins)

# Second, for multi-config builds (e.g. msvc)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
  string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin/${OUTPUTCONFIG}/Plugins )
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib/${OUTPUTCONFIG}/Plugins )
  set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib/${OUTPUTCONFIG}/Plugins )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

#set the rpath to look up from the Plugin directory to the main lib folder
set(CMAKE_INSTALL_RPATH "$ORIGIN/..")

add_subdirectory(device_sdk)
add_subdirectory(openni_sensor)

if (ASTRA_SKELETON)
  add_subdirectory(orbbec_skeleton)
endif()

if (ASTRA_HAND)
  add_subdirectory(orbbec_hand)
endif()

if (ASTRA_XS)
  add_subdirectory(orbbec_xs)
endif()

if (NOT ASTRA_ANDROID)

  if (ASTRA_MOCK_DEVICE)
    add_subdirectory(mock_sensor)
  endif()

  if (ASTRA_STREAMPLAYER)
    add_subdirectory(orbbec_streamplayer)
  endif()

endif()
