battleplanets : Body.o main.o
	g++ -std=c++11 -Wall main.o Body.o -lncurses -o main

Body.o : Body.cpp Body.h
	g++ -std=c++11 -Wall Body.cpp -c

main.o : main.cpp Body.h
	g++ -std=c++11 -Wall main.cpp -lncurses -c


clean:
	rm -f *.o bodytest
