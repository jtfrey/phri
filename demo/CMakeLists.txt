#
# Test the functionality of the ALU microunit.
#
add_executable(alu_test)
target_sources(alu_test
    PRIVATE
        alu_test.c)
target_link_libraries(alu_test
    PRIVATE
        phri)


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

#
# Demo program that runs a PHRI program that estimates π/4 in
# Q1.14 fixed-point format.
#
add_executable(pi_over_4)
target_sources(pi_over_4
    PRIVATE
        pi_over_4.c)
target_link_libraries(pi_over_4
    PRIVATE
        phri)

