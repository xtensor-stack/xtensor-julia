############################################################################
# Copyright (c) Johan Mabille and Sylvain Corlay                           #
# Copyright (c) QuantStack                                                 #
#                                                                          #
# Distributed under the terms of the BSD 3-Clause License.                 #
#                                                                          #
# The full license is in the file LICENSE, distributed with this software. #
############################################################################

cmake_minimum_required(VERSION 3.1)
project(xtensor-julia)

set(XTENSOR_JULIA_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Versioning
# ==========

set(XTENSOR_JULIA_CONFIG_FILE
    "${XTENSOR_JULIA_INCLUDE_DIR}/xtensor-julia/xtensor_julia_config.hpp")
file(STRINGS ${XTENSOR_JULIA_CONFIG_FILE} xtensor_julia_version_defines
    REGEX "#define XTENSOR_JULIA_VERSION_(MAJOR|MINOR|PATCH)")
foreach(ver ${xtensor_julia_version_defines})
    if(ver MATCHES "#define XTENSOR_JULIA_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
        set(XTENSOR_JULIA_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
endforeach()
set(${PROJECT_NAME}_VERSION
    ${XTENSOR_JULIA_VERSION_MAJOR}.${XTENSOR_JULIA_VERSION_MINOR}.${XTENSOR_JULIA_VERSION_PATCH})
message(STATUS "xtensor-julia v${${PROJECT_NAME}_VERSION}")

# Dependencies
# ============

find_package(xtl 0.7.4 REQUIRED)
message(STATUS "Found xtl: ${xtl_INCLUDE_DIRS}/xtl")
find_package(xtensor 0.24.0 REQUIRED)
message(STATUS "Found xtensor: ${xtensor_INCLUDE_DIRS}/xtensor")
find_package(JlCxx 0.9.0 REQUIRED)
message(STATUS "Found JlCxx: ${JlCxx_INCLUDE_DIRS}")

# Build
# =====

set(XTENSOR_JULIA_HEADERS
    ${XTENSOR_JULIA_INCLUDE_DIR}/xtensor-julia/jlarray.hpp
    ${XTENSOR_JULIA_INCLUDE_DIR}/xtensor-julia/jlcontainer.hpp
    ${XTENSOR_JULIA_INCLUDE_DIR}/xtensor-julia/jltensor.hpp
    ${XTENSOR_JULIA_INCLUDE_DIR}/xtensor-julia/jlvectorize.hpp
    ${XTENSOR_JULIA_INCLUDE_DIR}/xtensor-julia/xtensor_julia_config.hpp
)

add_library(xtensor-julia INTERFACE)
target_include_directories(xtensor-julia INTERFACE
    "$<BUILD_INTERFACE:${XTENSOR_JULIA_INCLUDE_DIR}>"
    $<INSTALL_INTERFACE:include>)
target_link_libraries(xtensor-julia INTERFACE xtensor)
get_target_property(inc_dir xtensor-julia INTERFACE_INCLUDE_DIRECTORIES)

OPTION(BUILD_TESTS "xtensor test suite" OFF)
OPTION(DOWNLOAD_GTEST "build gtest from downloaded sources" OFF)

if(DOWNLOAD_GTEST OR GTEST_SRC_DIR)
    set(BUILD_TESTS ON)
endif()

if(BUILD_TESTS)
    include_directories(${XTENSOR_JULIA_INCLUDE_DIR})
    include_directories(${xtl_INCLUDE_DIRS})
    include_directories(${xtensor_INCLUDE_DIRS})
    include_directories(${JlCxx_INCLUDE_DIRS})
    add_subdirectory(test)
endif()

# Installation
# ============

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS xtensor-julia
        EXPORT ${PROJECT_NAME}-targets)

# Makes the project importable from the build directory
export(EXPORT ${PROJECT_NAME}-targets
       FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")

install(FILES ${XTENSOR_JULIA_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xtensor-julia)

set(XTENSOR_JULIA_CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" CACHE
    STRING "install path for xtensor-juliaConfig.cmake")


configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${XTENSOR_JULIA_CMAKECONFIG_INSTALL_DIR})

# xtensor-julia is header-only and does not depend on the architecture.
# Remove CMAKE_SIZEOF_VOID_P from xtensor-juliaConfigVersion.cmake so that an xtensor-juliaConfig.cmake
# generated for a 64 bit target can be used for 32 bit targets and vice versa.
set(_XTENSOR_CMAKE_SIZEOF_VOID_P ${CMAKE_SIZEOF_VOID_P})
unset(CMAKE_SIZEOF_VOID_P)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
                                 VERSION ${${PROJECT_NAME}_VERSION}
                                 COMPATIBILITY AnyNewerVersion)
set(CMAKE_SIZEOF_VOID_P ${_XTENSOR_CMAKE_SIZEOF_VOID_P})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION ${XTENSOR_JULIA_CMAKECONFIG_INSTALL_DIR})
install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}Targets.cmake
        DESTINATION ${XTENSOR_JULIA_CMAKECONFIG_INSTALL_DIR})

