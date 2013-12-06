int tun_open(char *dev, int flags);

int tun_read(int fd, char *buf, int len);
int tun_read2( int fd, char *buf, int len, uint32_t &sourceIp, uint32_t &destIp );

void tun_setup( char *dev, int mtu );
