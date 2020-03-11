CC=gcc
CFLAGS=-I/usr/local/Cellar/argp-standalone/1.3/include/
LDFLAGS=-L/usr/local/Cellar/argp-standalone/1.3/lib/ -largp
generator: generator.c
	$(CC) $(CFLAGS) -o run_worms generator.c $(LDFLAGS)

