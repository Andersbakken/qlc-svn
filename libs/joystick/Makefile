all:
	gcc -c -fPIC -D__DEBUG__ libjoy.c
	gcc -shared -fPIC -lpthread -o libjoy.so libjoy.o
	gcc test.c -lpthread -o test -ldl

clean:
	rm -f *.o *.so *~ test
