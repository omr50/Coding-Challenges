all: output

output: main.o Object.o KeyVal.o val.o 
	g++ -o output main.o Object.o KeyVal.o val.o  

main.o: ../main.cpp
	g++ -c -o main.o ../main.cpp

Object.o: ../Classes/Object.cpp ../headers/Object.h
	g++ -c -o Object.o ../Classes/Object.cpp	

KeyVal.o: ../Classes/KeyVal.cpp ../headers/KeyVal.h
	g++ -c -o KeyVal.o ../Classes/KeyVal.cpp	

val.o: ../Classes/val.cpp ../headers/val.h
	g++ -c -o val.o ../Classes/val.cpp	


# output, o files are in build so no path.
# main.cpp is back one directory, ./
# building from build directory, so use the
# ../ notation for Classes path and header path
# preprocessor directives already include .h files
# but makefile allows you to check if any of those
# files were updated and only re-compile the ones
# that need it.