all: main

main:
	gcc -c tun_dev.c
	g++ -std=c++11 -c tun.cpp
	g++ -std=c++11 -c client.cpp
	g++ -std=c++11 -c facebook.cpp
	g++ -std=c++11 -o facebook-tunnel main.cpp tun_dev.o tun.o client.o facebook.o -lcurl

clean:
	rm facebook-tunnel
	rm *.o
