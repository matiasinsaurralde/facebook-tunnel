all: facebook-tunnel

facebook-tunnel:
	g++ -o facebook-tunnel main.cpp tun.c
