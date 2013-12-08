LDFLAGS = `sh osflags ld $(MODE)`
CFLAGS = -c -g `sh osflags c $(MODE)`
TUN_DEV_FILE = `sh osflags dev $(MODE)`
GCC = gcc
GPP = g++

.PHONY: directories

all: hans

directories: build_dir

build_dir:
	mkdir -p build

tunemu.o: directories build/tunemu.o

hans: directories build/tun.o build/sha1.o build/main.o build/client.o build/server.o build/auth.o build/worker.o build/time.o build/tun_dev.o build/echo.o build/fb.o build/exception.o build/utility.o
	$(GPP) -o hans build/tun.o build/sha1.o build/main.o build/client.o build/server.o build/auth.o build/worker.o build/time.o build/tun_dev.o build/echo.o build/fb.o build/exception.o build/utility.o $(LDFLAGS)

build/utility.o: src/utility.cpp src/utility.h
	$(GPP) -c src/utility.cpp -o $@ -o $@ $(CFLAGS)

build/exception.o: src/exception.cpp src/exception.h
	$(GPP) -c src/exception.cpp -o $@ $(CFLAGS)

build/echo.o: src/echo.cpp src/echo.h src/exception.h
	$(GPP) -c src/echo.cpp -o $@ $(CFLAGS)

build/fb.o: src/fb.cpp src/fb.h src/exception.h src/utility.h
	$(GPP) -c src/fb.cpp -o $@ $(CFLAGS)

build/tun.o: src/tun.cpp src/tun.h src/exception.h src/utility.h src/tun_dev.h
	$(GPP) -c src/tun.cpp -o $@ $(CFLAGS)

build/tun_dev.o:
	$(GCC) -c $(TUN_DEV_FILE) -o build/tun_dev.o -o $@ $(CFLAGS)

build/sha1.o: src/sha1.cpp src/sha1.h
	$(GPP) -c src/sha1.cpp -o $@ $(CFLAGS)

build/main.o: src/main.cpp src/client.h src/server.h src/exception.h src/worker.h src/auth.h src/time.h src/echo.h src/tun.h src/tun_dev.h
	$(GPP) -c src/main.cpp -o $@ $(CFLAGS)

build/client.o: src/client.cpp src/client.h src/server.h src/exception.h src/config.h src/worker.h src/auth.h src/time.h src/echo.h src/tun.h src/tun_dev.h
	$(GPP) -c src/client.cpp -o $@ $(CFLAGS)

build/server.o: src/server.cpp src/server.h src/client.h src/utility.h src/config.h src/worker.h src/auth.h src/time.h src/echo.h src/tun.h src/tun_dev.h
	$(GPP) -c src/server.cpp -o $@ $(CFLAGS)

build/auth.o: src/auth.cpp src/auth.h src/sha1.h src/utility.h
	$(GPP) -c src/auth.cpp -o $@ $(CFLAGS)

build/worker.o: src/worker.cpp src/worker.h src/tun.h src/exception.h src/time.h src/echo.h src/tun_dev.h src/config.h
	$(GPP) -c src/worker.cpp -o $@ $(CFLAGS)

build/time.o: src/time.cpp src/time.h
	$(GPP) -c src/time.cpp -o $@ $(CFLAGS)

clean:
	rm -f build/tun.o build/sha1.o build/main.o build/client.o build/server.o build/auth.o build/worker.o build/time.o build/tun_dev.o build/echo.o build/exception.o build/utility.o build/tunemu.o hans
	rm -df build

build/tunemu.o: src/tunemu.h src/tunemu.c
	$(GCC) -c src/tunemu.c -o build/tunemu.o
