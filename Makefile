SRC = main.c ws_server.c http_server.c
MAIN = picar.bin
CFLAGS += -g -fno-builtin-log -I/usr/local/include
LDFLAGS += -L/usr/local/lib64 -lws

all: $(MAIN)

$(MAIN): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRC) -o $@

clean:
	rm -f $(MAIN)
