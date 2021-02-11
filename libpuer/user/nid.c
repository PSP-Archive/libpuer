#include "../libpuer.h"

#if 0
typedef int size_t;
typedef unsigned char u8;
typedef unsigned int u32;
#include "../include/sha1.h"

unsigned int read32(const void *p){
	const unsigned char *x=(const unsigned char*)p;
	return x[0]|(x[1]<<8)|(x[2]<<16)|(x[3]<<24);
}
#endif

u32 toNid(const char *name){
	unsigned char digest[16];
	struct sha1_ctxt sha1ctx;
	sha1_init(&sha1ctx);
	sha1_loop(&sha1ctx,(u8*)name,strlen(name));
	sha1_result(&sha1ctx,digest);
	return read32(digest);
}

//main(){printf("%08x\n",toNid("kuKernelGetModel"));}
