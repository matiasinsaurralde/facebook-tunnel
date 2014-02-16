CC=gcc
CFLAGS=
CXX=g++
CXXFLAGS=-std=c++0x

all: main

main:
	$(CC) $(CFLAGS) -c tun_dev.c
	$(CXX) $(CXXFLAGS) -c facebook.cpp
	$(CXX) $(CXXFLAGS) -c tun.cpp base64encode.c
	$(CXX) $(CXXFLAGS) -c client.cpp
	$(CXX) $(CXXFLAGS) -c utils.cpp
	$(CXX) $(CXXFLAGS) -o facebook-tunnel main.cpp base64encode.o tun_dev.o tun.o client.o facebook.o utils.o -lcurl -lgumbo -pthread

clean:
	rm facebook-tunnel
	rm *.o
