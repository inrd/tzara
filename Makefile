CC = gcc
LINKER = gcc
CFLAGS = -Wall -Wextra -O2
OBJECTS = nodes.o \
		  tzara.o \
		  parser.o \
		  main.o

default: tzara
tzara: $(OBJECTS)
	$(LINKER) $(OBJECTS) -o tzara 
.o: $*.c
	$(CC) $(CFLAGS) $*.c
clean:
	rm -rf *.o tzara
install:
	cp tzara /usr/local/bin/tzara

