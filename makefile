#makefile
ghc: ghc.o ghcontrol.o pisensehat.o
	gcc -g -o ghc ghc.o ghcontrol.o pisensehat.o -lwiringPi
#	gcc -g -o ghc ghc.o ghcontrol.o pisensehat.o -lpython2.7
ghc.o: ghc.c ghcontrol.h
	gcc -g -c ghc.c
ghcontrol.o: ghcontrol.c ghcontrol.h
	gcc -g -c ghcontrol.c
pisensehat.o: pisensehat.c pisensehat.h
	gcc -g -c pisensehat.c
clean:
	touch *
	rm *.o
