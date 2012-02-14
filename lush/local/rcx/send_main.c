
int
main (int argc, char **argv) {
    unsigned char sendbuf[BUFFERSIZE];
    unsigned char recvbuf[BUFFERSIZE];
    unsigned char *sp = sendbuf;
    char *tty;
    int len;
    int fd;
    int i;

    /* Print usage if nothing on command line */

    if (argc == 1) {
	fprintf(stderr, "usage: %s byte [byte ...]\n", argv[0]);
	exit(1);
    }

    /* Open the serial port */

    if ((tty = getenv("RCXTTY")) == NULL)
	tty = DEFAULTTTY;

    fd = rcx_init(tty);

    /* Assemble the message */

    for (i = 1; i < argc; i++) {
	*sp++ = strtol(argv[i], NULL, 16);
    }

    /* Send it */

    len = rcx_send(fd, sendbuf, sp - sendbuf, recvbuf, BUFFERSIZE);

    /* Close the tty */

    rcx_close(fd);
    
    /* Print response and exit */

    if (len == -2) {
	fprintf(stderr, "%s: Bad link.\n", argv[0]);
	exit(4);
    }
    if (len == -1) {
	fprintf(stderr, "%s: Bad response.\n", argv[0]);
	exit(2);
    }
    if (len == 0) {
	fprintf(stderr, "%s: No response.\n", argv[0]);
	exit(3);
    }
    if (len > BUFFERSIZE) {
	fprintf(stderr, "%s: Response too large.\n", argv[0]);
	exit(5);
    }

    hexdump(recvbuf, len);

    return 0;
}
