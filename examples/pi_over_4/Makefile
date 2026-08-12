#
# 64-bit variant for Apple ARM64
#

pi_over_4.arm64: pi_over_4.arm64.o
	ld -o pi_over_4.arm64 pi_over_4.arm64.o -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path` -e _main -arch arm64 

pi_over_4.arm64.o: pi_over_4.arm64.s
	as -arch arm64 -o pi_over_4.arm64.o -g pi_over_4.arm64.s

dump:
	objdump -s -d pi_over_4.arm64.o

ndump:
	objdump -s -d -M no-aliases pi_over_4.arm64.o

