// Cert stuff

char *makekey(void);
char *makecert(const char *keyder, const char *cakeyder, const char *cacertder, const char *name);
EVP_PKEY *der2pkey(const char *der);
X509 *der2x509(const char *der);
char *certpem(const char *der);
char *keypem(const char *der);
