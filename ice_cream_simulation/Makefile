default:
	gcc -o compiled.o ice_cream.c -lpthread
	chmod u+x compiled.o

test:
	make clean
	make
	clear
	./compiled.o

clean:
	rm -f *.o