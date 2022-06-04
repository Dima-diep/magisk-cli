CFLAGS = -lsqlite3 -static -pthread -ldl
arm64exec:
	aarch64-linux-gnu-gcc $(CFLAGS) magisk-cli.cpp -o magisk-cli
	aarch64-linux-gnu-strip magisk-cli

armhfexec:
	arm-linux-gnueabihf-gcc $(CFLAGS) magisk-cli.cpp -o magisk-cli
	arm-linux-gnueabihf-strip magisk-cli
