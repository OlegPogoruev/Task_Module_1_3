#цель: зависимость
#[tab] команда


BIN = userspace_proc
SOURCES = userspace_proc.c
HEADERS = ProcFile.h
CC := gcc

all:
	$(CC) $(SOURCES) -o $(BIN).out
$(BIN): $(HEADERS)

run: all
	sudo ./$(BIN).out

#clean:
#	rm -rf $(BIN) *.o $(BIN).out



















