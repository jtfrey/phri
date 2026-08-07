#
# CMake infrastructure for the library
#

find_package(Threads REQUIRED)

#
# Generate our common header file and have all targets in this
# directory use the binary directory on the search path:
#
configure_file(phri.h.in phri.h)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Source files for the library:
set(LIBPHRI_SOURCES
    phri_mem.c
    phri_cpu.c
    phri_bus.c)

# Header files for the library:
set(LIBPHRI_HEADERS
    phri_mem.h
    phri_cpu.h
    phri_bus.h
    phri_buslink.h
    phri_alu.h
    phri_branch.h
    phri_data.h
    phri_instr.h
    phri_math.h
    phri_statusbits.h)

add_library(phri SHARED)
target_sources(phri
    PRIVATE
        ${LIBPHRI_SOURCES}

    PUBLIC
        FILE_SET static_headers
        TYPE HEADERS
        FILES
            ${LIBPHRI_HEADERS}

    PUBLIC
        FILE_SET generated_headers
        TYPE HEADERS
        BASE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/phri.h
)
target_link_libraries(phri
    PUBLIC
        Threads::Threads)
