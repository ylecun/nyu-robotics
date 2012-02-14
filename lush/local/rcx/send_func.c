/*
 *  send.c
 *
 *  A program to communicate with an RCX.
 *
 *  Under IRIX, Linux, and Solaris, you should be able to compile this
 *  program with cc send.c -o send.  I don't know about other versions of
 *  Unix, although I'd be interested in hearing about compatibility issues
 *  that you are able to fix.
 *
 *  Set DEFAULTTTY to the serial device you want to use.
 *  Set the RCXTTY environment variable to override DEFAULTTTY.
 *
 *  Some additional documentation is available at:
 *
 *     http://graphics.stanford.edu/~kekoa/rcx/tools.html
 *
 *  Acknowledgements:
 *
 *     Ray Kelm sent word that send.c compiles successfully under Linux,
 *        and he passed on info about the names of the serial devices.
 *
 *     Laurent Demailly ran this through lint, and suggested some changes,
 *        including a printf bug fix.  He also pointed out some behavior
 *        that prompted me to increase the number of retries in the send
 *        code.  He also pointed out that this program compiles fine under
 *        Solaris.
 *
 *     Other people have used the ideas in this program (and Paul Haas's
 *        Perl script) to write similar programs in other languages.  More
 *        information is available at http://www.crynwr.com/lego-robotics/.
 */

/*  Copyright (C) 1998, Kekoa Proudfoot.  All Rights Reserved.
 *
 *  License to copy, use, and modify this software is granted provided that
 *  this notice is retained in any copies of any part of this software.
 *
 *  The author makes no guarantee that this software will compile or
 *  function correctly.  Also, if you use this software, you do so at your
 *  own risk.
 * 
 *  Kekoa Proudfoot
 *  kekoa@graphics.stanford.edu
 *  10/3/98
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <ctype.h>


#define DEFAULTTTY   "/dev/ttyS0" /* Linux - COM1 */
#define BUFFERSIZE   4096
#define RETRIES      5

/* Hexdump routine */

#define LINE_SIZE   16
#define GROUP_SIZE  4
#define UNPRINTABLE '.'

void
hexdump(void *buf, int len)
{
    unsigned char *b = (unsigned char *)buf;
    int i, j, w;
    
    for (i = 0; i < len; i += w) {
	w = len - i;
	if (w > LINE_SIZE)
	    w = LINE_SIZE;
	printf("%04x: ", i);
	for (j = 0; j < w; j++, b++) {
	    printf("%02x ", *b);
	    if ((j + 1) % GROUP_SIZE == 0)
		putchar(' ');
	}
#if 0
	b -= w;
	for ( ; j < LINE_SIZE; j++) {
	    putchar(' '); putchar(' '); putchar(' ');
	    if ((j + 1) % GROUP_SIZE == 0)
		putchar(' ');
	}
	for (j = 0; j < w; j++, b++) {
	    putchar(isprint(*b) ?(*b) : UNPRINTABLE);
	}
#endif
	putchar('\n');
    }
}

int
rcx_init(char *tty)
{
    int fd;
    struct termios ios;

    if ((fd = open(tty, O_RDWR)) < 0) {
	perror("open");
	exit(1);
    }

    if (!isatty(fd)) {
	close(fd);
	fprintf(stderr, "%s: not a tty\n", tty);
	exit(1);
    }

    memset(&ios, 0, sizeof(ios));
    ios.c_cflag = CREAD | CLOCAL | CS8 | PARENB | PARODD;
    cfsetispeed(&ios, B2400);
    cfsetospeed(&ios, B2400);

    if (tcsetattr(fd, TCSANOW, &ios) == -1) {
	perror("tcsetattr");
	exit(1);
    }

    return fd;
}

void
rcx_close(int fd)
{
    close(fd);
}

int
rcx_send(int fd, unsigned char *sbuf, int slen, unsigned char *rbuf, int rlen)
{
    unsigned char tbuf[BUFFERSIZE];
    unsigned char vbuf[BUFFERSIZE];
    unsigned char *sp = sbuf;
    struct timeval tv;
    fd_set fds;
    int tlen = 0, vlen, vpos, rpos;
    int sum = 0, retry, returnval, count;

    tbuf[tlen++] = 0x55;
    tbuf[tlen++] = 0xff;
    tbuf[tlen++] = 0x00;
    while (slen--) {
	tbuf[tlen++] = *sp;
	tbuf[tlen++] = (~*sp) & 0xff;
	sum += *sp++;
    }
    tbuf[tlen++] = sum;
    tbuf[tlen++] = ~sum;

    for (retry = 0; retry < RETRIES; retry++) {
	if (write(fd, tbuf, tlen) != tlen) {
	    perror("write");
	    exit(1);
	}

	vlen = 0;
	while (1) {
	    FD_ZERO(&fds);
	    FD_SET(fd, &fds);
	    tv.tv_sec = 0;
	    tv.tv_usec = 300000;
	    if (select(FD_SETSIZE, &fds, NULL, NULL, &tv) == -1) {
		perror("select");
		exit(1);
	    }
	    if (!FD_ISSET(fd, &fds))
		break;
	    if ((count = read(fd, &vbuf[vlen], BUFFERSIZE - vlen)) == -1) {
		perror("read");
		exit(1);
	    }
	    vlen += count;
	}

	/* Check echo */

	returnval = -2;
	if (vlen < tlen)
	    continue; /* retry */
	for (vpos = 0; vpos < tlen; vpos++)
	    if (tbuf[vpos] != vbuf[vpos])
		break;
	if (vpos < tlen)
	    continue; /* retry */

	/* Check reply */

	returnval = 0;
	if (vpos == vlen)
	    continue;

	returnval = -1;
	if (vlen - vpos < 5)
	    break; /* could continue instead */

	if (vbuf[vpos++] != 0x55)
	    break; /* could continue instead */
	if (vbuf[vpos++] != 0xff)
	    break; /* could continue instead */
	if (vbuf[vpos++] != 0x00)
	    break; /* could continue instead */

	for (sum = 0, rpos = 0; vpos < vlen - 2; vpos += 2, rpos++) {
	    if (vbuf[vpos] != ((~vbuf[vpos+1]) & 0xff))
		break;
	    sum += vbuf[vpos];
	    if (rpos < rlen)
		rbuf[rpos] = vbuf[vpos];
	}
	if (vpos != vlen - 2)
	    break; /* could continue instead */
	if (vbuf[vpos] != ((~vbuf[vpos+1]) & 0xff))
	    break; /* could continue instead */
	if ((sum & 0xff) != vbuf[vpos])
	    break; /* could continue instead */

	return rpos;
    }

    return returnval;
}
