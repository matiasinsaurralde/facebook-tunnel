CC=gcc
CXX=g++

all: main

main:
	$(CC) -c tun_dev.c
	$(CXX) -std=c++11 -c facebook.cpp
	$(CXX) -std=c++11 -c tun.cpp base64encode.c
	$(CXX) -std=c++11 -c client.cpp
	$(CXX) -std=c++11 -c utils.cpp
	$(CXX) -std=c++11 -o facebook-tunnel main.cpp base64encode.o tun_dev.o tun.o client.o facebook.o utils.o -lcurl -lgumbo -pthread

clean:
	rm facebook-tunnel
	rm *.o
