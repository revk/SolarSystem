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
#include <openssl/pem.h>
#include <openssl/x509v3.h>

#define KEYLEN  2048            // Default key length (we want to fit in an MQTT message)

char *makekey(void)
{                               // Make a key and return (malloc'd) PEM private key
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
   BIO *bio = BIO_new(BIO_s_mem());
   PEM_write_bio_RSAPrivateKey(bio, rsa, NULL, NULL, 0, NULL, NULL);
   int keylen = BIO_pending(bio);
   char *key = calloc(keylen + 1, 1);
   BIO_read(bio, key, keylen);
   BIO_free(bio);
   EVP_PKEY_free(pkey);
   return key;
}

char *makecert(const char *keypem, const char *cakeypem, const char *cacertpem, const char *name)
{                               // Make a cert and return (malloc'd) PEM - self signed if certpem is NULL
   EVP_PKEY *key = NULL,
       *cakey = NULL;
   X509 *cacert = NULL;
   if (keypem && *keypem)
   {
      FILE *k = fmemopen((void *) keypem, strlen(keypem), "r");
      key = PEM_read_PrivateKey(k, NULL, NULL, NULL);   // No password
      fclose(k);
   }
   if (cakeypem && *cakeypem)
   {
      FILE *k = fmemopen((void *) cakeypem, strlen(cakeypem), "r");
      cakey = PEM_read_PrivateKey(k, NULL, NULL, NULL); // No password
      fclose(k);
   }
   if (cacertpem && *cacertpem)
   {
      FILE *k = fmemopen((void *) cacertpem, strlen(cacertpem), "r");
      cacert = PEM_read_X509(k, NULL, NULL, NULL);
      fclose(k);
   }
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
   if(!cacertpem||!*cacertpem)
   nz(X509_add_ext(cert,X509V3_EXT_conf_nid(NULL,&ctx,NID_basic_constraints,(char*)"critical, CA:TRUE"),-1));
   nz(X509_add_ext(cert,X509V3_EXT_conf_nid(NULL,&ctx,NID_subject_key_identifier,(char*)"hash"),-1));
#if 0
           X509v3 extensions:
            X509v3 Subject Key Identifier:
                32:94:4A:8E:BA:F8:2A:2E:EA:7E:6A:BE:8A:E0:CC:FD:B4:AE:A0:38
            X509v3 Authority Key Identifier:
                keyid:32:94:4A:8E:BA:F8:2A:2E:EA:7E:6A:BE:8A:E0:CC:FD:B4:AE:A0:38

            X509v3 Basic Constraints: critical
                CA:TRUE
#endif

   // Sign
   nz(X509_sign(cert, cakey ? : key, EVP_sha256()));
   // Write cert
   BIO *bio = BIO_new(BIO_s_mem());
   PEM_write_bio_X509(bio, cert);
   int keylen = BIO_pending(bio);
   char *pem_cert = calloc(keylen + 1, 1);
   BIO_read(bio, pem_cert, keylen);
   BIO_free(bio);
   X509_free(cert);
   if (cacert)
      X509_free(cacert);
   if (key)
      EVP_PKEY_free(key);
   if (cakey)
      EVP_PKEY_free(cakey);
#undef nz
   return pem_cert;
}
