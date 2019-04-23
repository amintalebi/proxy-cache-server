all: server

clean:
	@rm -rf *.o
	@rm -rf server

server: main.o httpd.o
	gcc -o server $^ -lcurl

main.o: main.c httpd.h
	gcc -c -o main.o main.c -lcurl

httpd.o: httpd.c httpd.h
	gcc -c -o httpd.o httpd.c -lcurl

