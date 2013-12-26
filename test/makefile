all: clean parse req_test replay read

parse:
	gcc parse.c -o parse

req_test:
	gcc req_test.c -o req_test -lcurl

replay:
	gcc replay.c -o replay

read:
	g++ read.cpp -o read -lcurl -lgumbo

clean:
	rm -f req_test parse replay read
