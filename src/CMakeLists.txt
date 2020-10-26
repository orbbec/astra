# add subdirs

include_directories(${ASTRA_INCLUDE_DIR})

set(CMAKE_DEBUG_POSTFIX "-d")

include(InstallPDB)

#set the rpath to look in the same directory for other Astra libs
set(CMAKE_INSTALL_RPATH "$ORIGIN")

add_subdirectory(astra_core)
add_subdirectory(astra)
add_subdirectory(astra_core_api)
add_subdirectory(plugins)

set (_projname "ProxyPreprocessor")

add_custom_target(${_projname} ALL
  COMMENT "=== LPP ==="
  COMMAND ${CLISP_EXECUTABLE} "lpp.cl" "../include"
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/lpp/"
  COMMAND ${CLISP_EXECUTABLE} "lpp.cl" "../src"
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/lpp/")

add_dependencies(astra_core ${_projname})
add_dependencies(astra ${_projname})
add_dependencies(astra_core_api ${_projname})
add_dependencies(openni_sensor ${_projname})

add_dependencies(${SDK_DEPENDENT_TARGET} astra_core)
add_dependencies(${SDK_DEPENDENT_TARGET} astra)
add_dependencies(${SDK_DEPENDENT_TARGET} astra_core_api)
add_dependencies(${SDK_DEPENDENT_TARGET} openni_sensor)

if (ASTRA_XS AND ASTRA_HAND)
  add_dependencies(${SDK_DEPENDENT_TARGET} orbbec_hand)
  add_dependencies(orbbec_hand ${_projname})
endif()

if (ASTRA_XS)
  add_dependencies(${SDK_DEPENDENT_TARGET} orbbec_xs)
  add_dependencies(orbbec_xs ${_projname})
endif()

add_dependencies(${SDK_DEPENDENT_TARGET} copytoml_astra)

if (ASTRA_HAND)
  add_dependencies(${SDK_DEPENDENT_TARGET} copytoml_hand)
endif()