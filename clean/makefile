all: main

main:
	gcc -c tun_dev.c
	g++ -c tun.cpp
	g++ -c client.cpp
	g++ -c facebook.cpp
	g++ -o facebook-tunnel main.cpp tun_dev.o tun.o client.o facebook.o

clean:
	rm facebook-tunnel
	rm *.o
