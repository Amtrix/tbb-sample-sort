
all:
	g++ -std=c++11 -o sort.out sort.cpp -ltbb -lgflags

clean:
	rm code.out
