#цель: зависимость
#[tab] команда


BIN = userspace_sys
SOURCES = userspace_sys.c
HEADERS = SysFile.h
CC := gcc

all:
	$(CC) $(SOURCES) -o $(BIN).out
$(BIN): $(HEADERS)

run: all
	sudo ./$(BIN).out

#clean:
#	rm -rf $(BIN) *.o $(BIN).out



















