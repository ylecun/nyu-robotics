
NAME = roombacmd
VERSION = 1.0
DISTNAME = $(NAME)-$(VERSION)


CFLAGS = -Wall -I.
CC = gcc

BINARIES = roombacmd simpletest
#BINARIES = simpletest

all: $(BINARIES)

clean:
	rm -f $(BINARIES)
	rm -f $(DISTNAME).tar.gz

.c.o:
	$(CC) $(CFLAGS) -o $@ $@.c 

simpletest: simpletest.c roombalib.c
	$(CC) $(CFLAGS) $^ -o $@ 

roombacmd: roombacmd.c roombalib.c
	$(CC) $(CFLAGS) $^ -o $@ 


dist:
	echo "Building dist $(DISTNAME).tar.gz"
	tar -C .. --exclude=*.tar.gz --exclude=*.svn --exclude=*~ \
        -cvzf $(DISTNAME).tar.gz $(NAME)
