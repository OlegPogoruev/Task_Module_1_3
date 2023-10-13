#цель: зависимость
#[tab] команда


BIN = userspace_ioctl
SOURCES = userspace_ioctl.c
HEADERS = chardev.h
CC := gcc

all:
	$(CC) $(SOURCES) -o $(BIN).out
$(BIN): $(HEADERS)

run: all
	sudo ./$(BIN).out

#clean:
#	rm -rf $(BIN) *.o $(BIN).out



















