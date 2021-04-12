SRC = main.c ws_server.c http_server.c /usr/local/lib/libws.a
MAIN = picar.bin
CFLAGS += -g -fno-builtin-log -I/usr/local/include
LDFLAGS += -lpthread

all: $(MAIN)

$(MAIN): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRC) -o $@

clean:
	rm -f $(MAIN)
