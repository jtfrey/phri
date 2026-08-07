#
# The base CMake build system configuration file for the project.
#
cmake_minimum_required(VERSION 3.10)
project(PHRI
    VERSION
        0.0.1
    LANGUAGES
        C)

#
# Which OS?
#
if(APPLE)
    set(PHRI_OS_MACOSX true)
elseif(UNIX)
    set(PHRI_OS_LINUX true)
else()
    message(FATAL "Unsupported operating system.")
endif()

add_subdirectory(libphri)
add_subdirectory(demo)