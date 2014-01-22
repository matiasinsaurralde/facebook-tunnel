all: main

main:
	gcc -c tun_dev.c
	g++ -std=c++11 -c facebook.cpp
	g++ -std=c++11 -c tun.cpp base64encode.c
	g++ -std=c++11 -c client.cpp
	g++ -std=c++11 -c utils.cpp
	g++ -std=c++11 -o facebook-tunnel main.cpp base64encode.o tun_dev.o tun.o client.o facebook.o utils.o -lcurl -lgumbo -pthread

clean:
	rm facebook-tunnel
	rm *.o
