cmake_minimum_required(VERSION 3.22)
project(AsyncFrameworkLib LANGUAGES CXX)

set(LIB_NAME AsyncFramework)

add_library(AsyncFramework
    src/AsyncNode.cpp
)

target_include_directories(AsyncFramework PRIVATE include)

set_target_properties(AsyncFramework
    PROPERTIES
    CMAKE_CXX_STANDARD 20
    CMAKE_CXX_STANDARD_REQUIRED ON
    CMAKE_CXX_EXTENSIONS OFF
)

# cache variables for installation destinations
include(GNUInstallDirs)

target_include_directories(AsyncFramework
    PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

install(TARGETS AsyncFramework
    EXPORT AsyncFrameworkTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY 
    include/async_framework
    include/threads
    include/common 
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT AsyncFrameworkTargets
    FILE AsyncFrameworkTargets.cmake
    NAMESPACE AsyncFramework::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AsyncFramework
)

# include the package config helpers and generate the file below
include(CMakePackageConfigHelpers)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/AsyncFrameworkConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AsyncFramework
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/AsyncFrameworkConfig.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AsyncFramework
)
