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

#
# Demo program that runs a PHRI program that calculates 1/n in
# Q1.14 fixed-point format.
#
add_executable(one_over_n)
target_sources(one_over_n
    PRIVATE
        one_over_n.c)
target_link_libraries(one_over_n
    PRIVATE
        phri)
