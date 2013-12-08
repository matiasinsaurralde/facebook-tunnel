all: clean parse req_test

parse:
	gcc parse.c -o parse

req_test:
	gcc req_test.c -o req_test -lcurl

clean:
	rm -f req_test parse
