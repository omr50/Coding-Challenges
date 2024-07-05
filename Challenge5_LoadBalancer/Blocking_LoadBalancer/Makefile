all: output

output: main.o HTTPObject.o
	g++ -g -o output main.o HTTPObject.o

HTTPObject.o: HTTPObject.cpp HTTPObject.h
	g++ -c -o HTTPObject.o HTTPObject.cpp

main.o: main.cpp
	g++ -c -o main.o ./main.cpp

