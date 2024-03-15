###############################################################################
### test-sgx
###
### A command line based SGX feature enumeration tool
###
### @see     https://www.gnu.org/software/make/manual/make.html
###
### @file    Makefile
### @author  Mark Nelson marknels@hawaii.edu>
###############################################################################

TARGET=test-sgx

test-sgx: cpuid.c test-sgx.c rdmsr.c vdso.c xsave.c
	gcc -Wl,--no-as-needed -Wall -Wextra -Wpedantic -masm=intel -o ${TARGET} -lcap $^

test: ${TARGET}
	./${TARGET}
	
clean:
	rm -fr ${TARGET} *.o *.obj *.exe
