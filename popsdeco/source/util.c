#include "../../libpuer/libpuer.h"
#include "md5.h"

#define printf pspDebugScreenPrintf

static char path[768];
static char *path_file;
static char line[768];
void check(const char *md5sum_name){
	char remote_md5[33];remote_md5[32]=0;
	char md5[33];md5[32]=0;
	unsigned char digest[16];
	FILE *f;
	int i,n=0,readlen;
	strcpy(myfile,"path.txt");
	f=fopen(mypath,"rb");
	myfgets(path,768,f);
    if(!*path||path[strlen(path)-1]!='/')
		strcat(path,"/");
	path_file=path+strlen(path);
	fclose(f);
	strcpy(myfile,md5sum_name);
	f=fopen(mypath,"rb");
	for(;myfgets(line,768,f);){
		sscanf(line,"%s%s",remote_md5,path_file);
		printf("%s: ",path_file);
		int fd=sceIoOpen(path,PSP_O_RDONLY,0777);
		if(fd<0){
			printf("Not found\n");
			n++;
			continue;
		}
		MD5_CTX md5ctx;
		MD5Init(&md5ctx);
		for(;(readlen=sceIoRead(fd,libpuer_buf,BUFLEN))>0;)MD5Update(&md5ctx, libpuer_buf, readlen);
		MD5Final(digest,&md5ctx);
		for(i=0;i<16;i++){
			int x=digest[i]>>4,y=digest[i]&0xf;
			md5[2*i+0]=x>9?(x-10+'a'):(x+'0');
			md5[2*i+1]=y>9?(y-10+'a'):(y+'0');
		}
		sceIoClose(fd);
		if(strcmp(md5,remote_md5)){
			printf("NG\n");
			n++;
			continue;
		}
		printf("OK\n");
	}
	fclose(f);
}
