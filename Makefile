semantic:scanner/scanner.o parser/parser.o main.o
	g++ -o semantic scanner.o parser.o main.o -lgdi32
scanner/scanner.o:scanner/scanner.cpp
	g++ -c scanner/scanner.cpp
parser/parser.o:parser/parser.cpp
	g++ -c parser/parser.cpp
main.o:main.cpp
	g++ -c main.cpp