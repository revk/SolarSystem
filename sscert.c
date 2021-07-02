// Certificate stuff

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/bio.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include "AJL/ajl.h"

#define KEYLEN  2048            // Default key length (we want to fit in an MQTT message)

EVP_PKEY *der2pkey(const char *der)
{
   uint8_t *buf;
   ssize_t len = j_base64d(der, &buf);
   if (len < 0)
      errx(1, "Bad DER %s", der);
   BIO *mem = BIO_new_mem_buf(buf, len);
   EVP_PKEY *key = d2i_PrivateKey_bio(mem, NULL);
   if (!key)
      errx(1, "Failed to make key from DER: %s",der);
   BIO_free(mem);
   free(buf);
   return key;
}

X509 *der2x509(const char *der)
{
   uint8_t *buf;
   ssize_t len = j_base64d(der, &buf);
   if (len < 0)
      errx(1, "Bad DER %s", der);
   BIO *mem = BIO_new_mem_buf(buf, len);
   X509 *x509 = d2i_X509_bio(mem, NULL);
   if (!x509)
      errx(1, "Failed to make cert from DER: %s",der);
   BIO_free(mem);
   free(buf);
   return x509;
}

char *certpem(const char *der)
{                               // Make a PEM
   X509 *cert = der2x509(der);
   char *buf;
   size_t len;
   FILE *fp = open_memstream(&buf, &len);
   PEM_write_X509(fp, cert);
   fclose(fp);
   X509_free(cert);
   return buf;
}

char *keypem(const char *der)
{                               // Make a PEM
   EVP_PKEY *key = der2pkey(der);
   char *buf;
   size_t len;
   FILE *fp = open_memstream(&buf, &len);
   PEM_write_PrivateKey(fp, key, NULL, NULL, 0, NULL, NULL);
   fclose(fp);
   EVP_PKEY_free(key);
   return buf;
}

char *makekey(void)
{                               // Make a key and return (malloc'd) DER private key
   EVP_PKEY *pkey;
   pkey = EVP_PKEY_new();
   BIGNUM *bn;
   bn = BN_new();
   BN_set_word(bn, RSA_F4);
   RSA *rsa;
   rsa = RSA_new();
   RAND_poll();
   RSA_generate_key_ex(rsa, KEYLEN, bn, NULL);
   EVP_PKEY_assign_RSA(pkey, rsa);
   uint8_t *buf = NULL;
   int len = i2d_PrivateKey(pkey, &buf);
   if (len < 0)
      errx(1, "Bad private key DER");
   char *key = strdup(j_base64(len, buf));
   free(buf);
   EVP_PKEY_free(pkey);
   return key;
}

char *makecert(const char *keyder, const char *cakeyder, const char *cacertder, const char *name)
{                               // Make a cert and return (malloc'd) DER - self signed if certder is NULL
   EVP_PKEY *key = NULL,
       *cakey = NULL;
   X509 *cacert = NULL;
   if (keyder && *keyder)
      key = der2pkey(keyder);
   if (cakeyder && *cakeyder)
      cakey = der2pkey(cakeyder);
   if (cacertder && *cacertder)
      cacert = der2x509(cacertder);
#define nz(x) do if(!(x))errx(1,"Failed %s",#x); while(0)
   // Make cert
   X509 *cert = X509_new();
   if (!cert)
      errx(1, "Failed to make cert");
   nz(X509_set_version(cert, 2));
   ASN1_INTEGER *aserial = ASN1_INTEGER_new();
   ASN1_INTEGER_set(aserial, time(0));
   nz(X509_set_serialNumber(cert, aserial));
   ASN1_INTEGER_free(aserial);
   X509_NAME *subject = X509_NAME_new();
   nz(X509_NAME_add_entry_by_txt(subject, "C", MBSTRING_ASC, (const unsigned char *) "UK", -1, -1, 0));
   nz(X509_NAME_add_entry_by_txt(subject, "O", MBSTRING_ASC, (const unsigned char *) "Solar System", -1, -1, 0));
   if (name && *name)
      nz(X509_NAME_add_entry_by_txt(subject, "CN", MBSTRING_ASC, (const unsigned char *) name, -1, -1, 0));
   nz(X509_set_subject_name(cert, subject));
   if (cacert)
      nz(X509_set_issuer_name(cert, X509_get_subject_name(cacert)));
   else
      nz(X509_set_issuer_name(cert, subject));  // Self signed
   X509_NAME_free(subject);
   nz(X509_set_pubkey(cert, key));
   nz(X509_gmtime_adj(X509_get_notBefore(cert), 0));
   nz(X509_gmtime_adj(X509_get_notAfter(cert), 50 * 365 * 86400));      // Long long time in the future

   X509V3_CTX ctx;
   X509V3_set_ctx_nodb(&ctx);
   X509V3_set_ctx(&ctx, cacert, cert, NULL, NULL, 0);
   if (!cacertder || !*cacertder)
      nz(X509_add_ext(cert, X509V3_EXT_conf_nid(NULL, &ctx, NID_basic_constraints, "critical, CA:TRUE"), -1));
   else
      nz(X509_add_ext(cert, X509V3_EXT_conf_nid(NULL, &ctx, NID_issuer_alt_name, "issuer:copy"), -1));
   nz(X509_add_ext(cert, X509V3_EXT_conf_nid(NULL, &ctx, NID_ext_key_usage, "critical, clientAuth, serverAuth"), -1));
   nz(X509_add_ext(cert, X509V3_EXT_conf_nid(NULL, &ctx, NID_subject_key_identifier, "hash"), -1));
#if 0
   X509v3 extensions:X509v3 Subject Key Identifier:32:94:4 A:8E: BA:F8:2 A:2E: EA:7E:6 A:BE:8 A:E0:CC:FD:B4:AE:A0:38 X509v3 Authority Key Identifier:keyid:32:94:4 A:8E: BA:F8:2 A:2E: EA:7E:6 A:BE:8 A:E0:CC:FD:B4:AE:A0:38 X509v3 Basic Constraints:critical CA:TRUE
#endif
       // Sign
    nz(X509_sign(cert, cakey ? : key, EVP_sha256()));
   // Write cert
   uint8_t *buf = NULL;
   int len = i2d_X509(cert, &buf);
   if (len < 0)
      errx(1, "Bad cert DER");
   char *der_cert = strdup(j_base64(len, buf));
   free(buf);
   X509_free(cert);
   if (cacert)
      X509_free(cacert);
   if (key)
      EVP_PKEY_free(key);
   if (cakey)
      EVP_PKEY_free(cakey);
#undef nz
   return der_cert;
}

#ifndef	LIB
		int main(int argc, const char *argv[])
{
	FILE *f;

	argc=argc;
	argv=argv;
	warnx("Testing certificate stuff");
	char *cakey=makekey();
	char *cacert=makecert(cakey, NULL, NULL, "SolarSystem");
	char *sitekey=makekey();
	char *sitecert=makecert(sitekey, cakey, cacert, "localhost");

	f=fopen("/tmp/cakey.pem","w");
	fprintf(f,"%s",keypem(cakey));
	fclose(f);
	f=fopen("/tmp/cacert.pem","w");
	fprintf(f,"%s",certpem(cacert));
	fclose(f);
	f=fopen("/tmp/sitekey.pem","w");
	fprintf(f,"%s",keypem(sitekey));
	fclose(f);
	f=fopen("/tmp/sitecert.pem","w");
	fprintf(f,"%s",certpem(sitecert));
	fclose(f);

	//system("openssl s_server -key /tmp/cakey.pem -cert /tmp/cacert.pem -accept localhost:4443");
	system("openssl s_server -key /tmp/sitekey.pem -cert /tmp/sitecert.pem -accept localhost:4443");

}
#endif
