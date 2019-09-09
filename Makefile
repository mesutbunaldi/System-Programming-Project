CC = gcc
DB = gdb
CFLAGS = -o
DFLAGS = -g
PROGNAME = 111044077_main
CLIENT=client

all:
	$(CC) -std=c11 -lpthread -lrt -lm -c $(PROGNAME).c 
	$(CC) $(PROGNAME).o -lpthread -lrt -lm $(CFLAGS) $(PROGNAME)
	$(CC) -std=c11 -lpthread -lrt -lm -c $(CLIENT).c 
	$(CC) $(CLIENT).o -lpthread -lrt -lm $(CFLAGS) $(CLIENT)

debug:
	$(CC) $(DFLAGS) -lpthread -lrt -lm $(PROGNAME).c $(CFLAGS) $(PROGNAME)
	$(DB) ./$(PROGNAME)

clean:
	rm -f $(PROGNAME) $(CLIENT) *.o 
