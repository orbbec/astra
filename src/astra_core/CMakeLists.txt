set (_projname "astra_core")

include_directories(
  ${CPPTOML_INCLUDE_DIR}
  ${TINYDIR_INCLUDE_DIR}
  ${EASYLOGGINGPP_INCLUDE_DIR}
  )

set(${_projname}_SOURCES
  ../../include/astra_core/astra_core.hpp
  ../../include/astra_core/DataStream.hpp
  ../../include/astra_core/Frame.hpp
  ../../include/astra_core/FrameListener.hpp
  ../../include/astra_core/plugins/PluginBase.hpp
  ../../include/astra_core/plugins/Plugin.hpp
  ../../include/astra_core/plugins/PluginServiceProxy.hpp
  ../../include/astra_core/plugins/SingleBinStream.hpp
  ../../include/astra_core/plugins/PluginStream.hpp
  ../../include/astra_core/plugins/StreamBin.hpp
  ../../include/astra_core/plugins/StreamCallbackListener.hpp
  ../../include/astra_core/StreamServiceProxy.hpp
  ../../include/astra_core/StreamDescription.hpp
  ../../include/astra_core/StreamReader.hpp
  ../../include/astra_core/StreamSet.hpp
  ../../include/astra_core/capi/astra_core.h
  ../../include/astra_core/capi/astra_defines.h
  ../../include/astra_core/capi/astra_streamservice_proxy.h
  ../../include/astra_core/capi/astra_types.h
  ../../include/astra_core/capi/astra_host_events.h
  ../../include/astra_core/capi/plugins/astra_plugin.h
  ../../include/astra_core/capi/plugins/astra_plugin_callbacks.h
  ../../include/astra_core/capi/plugins/astra_pluginservice_proxy.h
  astra_context.hpp
  astra_context.cpp
  astra_context_impl.hpp
  astra_context_impl.cpp
  astra_create_stream_proxy.hpp
  astra_create_plugin_proxy.hpp
  astra_configuration.hpp
  astra_configuration.cpp
  astra.cpp
  astra_stream.hpp
  astra_stream.cpp
  astra_stream_backend.hpp
  astra_stream_backend.cpp
  astra_plugin_service.hpp
  astra_plugin_service.cpp
  astra_plugin_service_impl.hpp
  astra_plugin_service_impl.cpp
  astra_plugin_service_delegate.hpp
  astra_plugin_manager.hpp
  astra_plugin_manager.cpp
  astra_parameter_bin.hpp
  astra_stream_service_delegate.hpp
  astra_streamset_catalog.hpp
  astra_streamset_catalog.cpp
  astra_streamset.hpp
  astra_streamset.cpp
  astra_streamset_connection.hpp
  astra_streamset_connection.cpp
  astra_stream_registered_event_args.hpp
  astra_stream_unregistering_event_args.hpp
  astra_logger.hpp
  astra_logger.cpp
  astra_logging.hpp
  astra_logging.cpp
  astra_signal.hpp
  astra_stream_connection.hpp
  astra_stream_connection.cpp
  astra_stream_bin.hpp
  astra_stream_bin.cpp
  astra_stream_reader.hpp
  astra_stream_reader.cpp
  astra_shared_library.hpp
  astra_registry.hpp
  astra_registry.cpp
  astra_filesystem.cpp
  astra_string.cpp
  )

set(${_projname}_ANDROID_SOURCES
  android/astra_environment_android.cpp
  unix/astra_filesystem_unix.cpp
  unix/astra_shared_library_unix.cpp
  )

set(${_projname}_UNIX_SOURCES
  unix/astra_environment_unix.cpp
  unix/astra_filesystem_unix.cpp
  unix/astra_shared_library_unix.cpp
  )

if(ASTRA_UNIX OR ASTRA_OSX)
  set(${_projname}_SOURCES ${${_projname}_SOURCES} ${${_projname}_UNIX_SOURCES})
endif()

set(${_projname}_SOURCES_GEN
  ../../include/astra_core/StreamServiceProxy.hpp.lpp
  ../../include/astra_core/capi/astra_core.h.lpp
  ../../include/astra_core/capi/astra_streamservice_proxy.h.lpp
  ../../include/astra_core/capi/plugins/astra_plugin_callbacks.h.lpp
  ../../include/astra_core/capi/plugins/astra_pluginservice_proxy.h.lpp
  ../../include/astra_core/plugins/PluginServiceProxy.hpp.lpp
  ../../include/astra_core/plugins/StreamCallbackListener.hpp.lpp
  astra.cpp.lpp
  astra_context.cpp.lpp
  astra_context.hpp.lpp
  astra_context_impl.hpp.lpp
  astra_create_plugin_proxy.hpp.lpp
  astra_create_stream_proxy.hpp.lpp
  astra_plugin_service.hpp.lpp
  astra_plugin_service_delegate.hpp.lpp
  astra_plugin_service_impl.hpp.lpp
  astra_stream_service_delegate.hpp.lpp
  )

source_group(templates FILES ${${_projname}_SOURCES_GEN})

add_definitions(-DASTRA_BUILD)

if (ANDROID)
  add_definitions(-DASTRA_ANDROID)
endif()

add_library(${_projname} SHARED ${${_projname}_SOURCES} ${${_projname}_SOURCES_GEN})

include_directories(${PROJECT_SOURCE_DIR}/src/astra_core_api)

target_link_libraries(${_projname} astra_core_api ${CMAKE_DL_LIBS})

add_subdirectory(tests)

add_custom_target(copytoml_astra ALL)

include(CopyIfNotExist)
copy_if_not_exist(copytoml_astra "astra.toml" "$<TARGET_FILE_DIR:${_projname}>/astra.toml")

#add_custom_target(copytoml_astra ALL
  #astra.toml
#  COMMAND ${CMAKE_COMMAND} -E copy
#  "${PROJECT_SOURCE_DIR}/src/astra_core/astra.toml"
#  "$<TARGET_FILE_DIR:${_projname}>")

set_target_properties(copytoml_astra PROPERTIES FOLDER CMakeCopyTargets)

install_lib(${_projname})
install_file("${PROJECT_SOURCE_DIR}/src/astra_core/astra.toml" lib)
