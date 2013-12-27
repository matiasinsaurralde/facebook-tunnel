all: clean parse req_test replay read login

parse:
	gcc parse.c -o parse

req_test:
	gcc req_test.c -o req_test -lcurl

replay:
	gcc replay.c -o replay

read:
	g++ read.cpp -o read -lcurl -lgumbo

login:
	g++ -std=c++11 login.cpp -o login -lcurl -lgumbo

clean:
	rm -f req_test parse replay read login
