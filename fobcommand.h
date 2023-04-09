#define KEY_DATA_LEN 18	// Type + Ver + Key
void *fobcommand(void *arg);
void randblock(unsigned char *, ssize_t);
void randkey(unsigned char[KEY_DATA_LEN]);
