#ifndef _MD5_H_
#define _MD5_H_

#ifdef WIN32
typedef unsigned int u_int32_t;
#else
#include <sys/types.h>
#endif 

#define MD5_HASHBYTES 16

typedef struct MD5Context {
	u_int32_t buf[4];
	u_int32_t bits[2];
	unsigned char in[64];
} MD5_CTX;

#ifdef __cplusplus
extern "C" {
#endif

extern void   MD5Init(MD5_CTX *context);
extern void   MD5Update(MD5_CTX *context, unsigned char const *buf,
	       unsigned len);
extern void   MD5Final(unsigned char digest[MD5_HASHBYTES], MD5_CTX *context);
extern void   MD5Transform(u_int32_t buf[4], u_int32_t const in[16]);
extern char * MD5End(MD5_CTX *, char *);
extern char * MD5File(const char *, char *);
extern char * MD5Data (const unsigned char *, unsigned int, char *);

#ifdef __cplusplus
}
#endif

#endif /* !_MD5_H_ */
