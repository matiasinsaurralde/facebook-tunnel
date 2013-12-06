.PHONY: msgpack

all:	msgpack facebook-tunnel
	@echo
	@echo ":)"

msgpack:
	@echo "=> Building/installing msgpack!"
	cd lib/msgpack-0.5.4 && ( ./configure --prefix=`pwd`/../../build && make && make install ) > /dev/null

facebook-tunnel:
	@echo "=> Building facebook-tunnel!"
	g++ -o facebook-tunnel main.cpp tun.c > /dev/null

clean:
	@echo "=> Cleanup"
	rm -rf build/*
	rm -f facebook-tunnel
