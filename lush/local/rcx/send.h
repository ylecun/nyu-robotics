void hexdump(void *buf, int len);
int rcx_init(char *tty);
void rcx_close(int fd);
int rcx_send(int fd, unsigned char *sbuf, int slen, unsigned char *rbuf, int rlen);
