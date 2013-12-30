all: main

main:
	gcc -c tun_dev.c
	gcc -c base64encode.c
	g++ -std=c++11 -c tun.cpp
	g++ -std=c++11 -c client.cpp
	g++ -std=c++11 -c facebook.cpp
	g++ -std=c++11 -c utils.cpp
	g++ -std=c++11 -o facebook-tunnel main.cpp tun_dev.o tun.o client.o facebook.o utils.o base64encode.o -lcurl -lgumbo

clean:
	rm facebook-tunnel
	rm *.o
