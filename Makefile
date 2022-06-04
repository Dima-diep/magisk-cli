CXXFLAGS := -lsqlite3 -static -pthread -ldl
arm64exec:
	aarch64-linux-gnu-g++ magisk-cli.cpp -o magisk-cli $(CXXFLAGS)
	aarch64-linux-gnu-strip magisk-cli

armhfexec:
	arm-linux-gnueabihf-g++ magisk-cli.cpp -o magisk-cli $(CXXFLAGS)
	arm-linux-gnueabihf-strip magisk-cli
