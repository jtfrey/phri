#
# Demo program that runs a PHRI program that does bit-shifted
# partial-sum multiplication.
#
add_executable(multiply)
target_sources(multiply
    PRIVATE
        multiply.c)
target_link_libraries(multiply
    PRIVATE
        phri)
