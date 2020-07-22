if(NOT MSVC)
    set(CMAKE_CXX_COMPILER_NAMES g++ clang++ icpc c++ cxx)
endif()
project(reckless LANGUAGES CXX)
CMAKE_MINIMUM_REQUIRED(VERSION 3.5)


################################################################################
# Add Flags
################################################################################
set(CMAKE_CXX_STANDARD 11)
if(MSVC)
    add_compile_options(/W4 /O3)
else()
    add_compile_options(-Wall -Wextra -O3 -g)
endif()

################################################################################
# Build Libraries
################################################################################
include_directories("reckless/include")
include_directories("performance_log/include")


set (SRC_LIST
    reckless/src/output_buffer.cpp
    reckless/src/ntoa.cpp
    reckless/src/template_formatter.cpp
    reckless/src/writer.cpp
    reckless/src/basic_log.cpp
    reckless/src/policy_log.cpp
    reckless/src/file_writer.cpp
    reckless/src/fd_writer.cpp
    reckless/src/mpsc_ring_buffer.cpp
    reckless/src/platform.cpp
    reckless/src/lockless_cv.cpp
    )

if(WIN32)
   set (SRC_LIST ${SRC_LIST}
   reckless/src/spsc_event_win32.cpp
   reckless/src/crash_handler_win32.cpp
   )
else()
   set (SRC_LIST ${SRC_LIST}
   reckless/src/crash_handler_unix.cpp
   )
endif()

add_library(reckless STATIC ${SRC_LIST})
set_target_properties(reckless PROPERTIES CXX_STANDARD 11)

# copy header files to staging area
file(COPY reckless/include DESTINATION ${CMAKE_BINARY_DIR} FILES_MATCHING PATTERN "*.h")
file(COPY reckless/include DESTINATION ${CMAKE_BINARY_DIR} FILES_MATCHING PATTERN "*.inl")
file(COPY reckless/include DESTINATION ${CMAKE_BINARY_DIR} FILES_MATCHING PATTERN "*.hpp")

################################################################################
# Add Dependencies
################################################################################
target_include_directories(reckless
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/reckless/include>
    $<INSTALL_INTERFACE:include>
    )


################################################################################
# Build Examples
################################################################################
message (STATUS "Making example applications")
add_subdirectory(examples)

################################################################################
# INSTALL
################################################################################
include(CMakePackageConfigHelpers)

if(NOT DEFINED CMAKE_INSTALL_PREFIX)
if(WIN32)
    set(CMAKE_INSTALL_PREFIX "C:/Program Files")
else(WIN32)
    set(CMAKE_INSTALL_PREFIX "/usr/local")
endif(WIN32)
endif(NOT DEFINED CMAKE_INSTALL_PREFIX)

install(TARGETS reckless
    EXPORT recklessTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION ${CMAKE_INSTALL_PREFIX}
    )

# this installs the include folder
install(DIRECTORY reckless/include DESTINATION ${CMAKE_INSTALL_PREFIX} FILES_MATCHING PATTERN "*.h")
install(DIRECTORY reckless/include DESTINATION ${CMAKE_INSTALL_PREFIX} FILES_MATCHING PATTERN "*.inl")
install(DIRECTORY reckless/include DESTINATION ${CMAKE_INSTALL_PREFIX} FILES_MATCHING PATTERN "*.hpp")

export(TARGETS reckless NAMESPACE reckless:: FILE recklessTargets.cmake)

# This generates recklessTargets.cmake
install(EXPORT recklessTargets
    FILE recklessTargets.cmake
    NAMESPACE reckless::
    DESTINATION lib/cmake/reckless
    )

# uninstall target
if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif(NOT TARGET uninstall)

