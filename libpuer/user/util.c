#include "../libpuer.h"

u8 libpuer_buf[BUFLEN];

//void __psp_libc_init(int argc, char *argv[]){ //re-hook it.
	// Initialize cwd from this program's path
	//__psp_init_cwd(argv[0]); //must not change cwd!

	// Initialize filedescriptor management
	//__psp_fdman_init();
//}

static int ExitCallback(int Arg1, int Arg2, void *Common){
	running = 0;
	return 0;
}
static int ExitCallback_Thread(SceSize Args, void *Argp){
	int CallbackId = sceKernelCreateCallback("ExitCallback", ExitCallback, NULL);
	sceKernelRegisterExitCallback(CallbackId);
	sceKernelSleepThreadCB();
	return 0;
}
int SetupExitCallback(){
	int ThreadId = sceKernelCreateThread("ExitCallback_Thread", ExitCallback_Thread, 0x11, 0xFA0, 0, 0);
	if(ThreadId >= 0)
		sceKernelStartThread(ThreadId, 0, 0);
	return ThreadId;
}
void ClearCaches(){
	int k1=pspSdkSetK1(0);
	sceKernelIcacheInvalidateAll();
	sceKernelDcacheWritebackInvalidateAll();
	pspSdkSetK1(k1);
}

u32 getNid(u32 HASH,u32 FW500,u32 FW620,u32 FW635,u32 FW660){
	u32 devkit = sceKernelDevkitVersion();
	if(devkit<PSP_FIRMWARE(0x500))return HASH;
	if(devkit<PSP_FIRMWARE(0x620))return FW500; //should be 0x555?
	if(devkit<PSP_FIRMWARE(0x630))return FW620;
	if(devkit<PSP_FIRMWARE(0x660))return FW635;
	return FW660;
}

u32 getNidIndex(){
	u32 devkit = sceKernelDevkitVersion();
	if(devkit<PSP_FIRMWARE(0x500))return 0;
	if(devkit<PSP_FIRMWARE(0x620))return 1; //should be 0x555?
	if(devkit<PSP_FIRMWARE(0x630))return 2;
	if(devkit<PSP_FIRMWARE(0x660))return 3;
	return 4;
}

unsigned int readAddr(void *mem){
	return *((unsigned int*)mem);
}

unsigned int readAddr24(void *mem){
	return (*((unsigned int*)mem))&0xffffff;
}

unsigned short readAddr16(void *mem){
	return (*((unsigned short*)mem));
}

unsigned char readAddr8(void *mem){
	return (*((unsigned char*)mem));
}

unsigned long long int readAddr64(void *mem){
	return (*((unsigned long long int*)mem));
}

void writeAddr(void *mem, const unsigned int value){
	*((unsigned int*)mem) = value;
}
/*
void writeAddr24(void *mem, const unsigned int value){
	*((unsigned int*)mem) = value&0xffffff;
}
*/

void writeAddr16(void *mem, const unsigned short value){
	*((unsigned short*)mem) = value;
}

void writeAddr8(void *mem, const unsigned char value){
	*((unsigned char*)mem) = value;
}

void writeAddr64(void *mem, const unsigned long long int value){
	*((unsigned long long int*)mem) = value;
}

unsigned int read32(const void *p){
	const unsigned char *x=(const unsigned char*)p;
	return x[0]|(x[1]<<8)|(x[2]<<16)|(x[3]<<24);
}

unsigned int read24(const void *p){
	const unsigned char *x=(const unsigned char*)p;
	return x[0]|(x[1]<<8)|(x[2]<<16);
}

unsigned short read16(const void *p){
	const unsigned char *x=(const unsigned char*)p;
	return x[0]|(x[1]<<8);
}

unsigned char read8(const void *p){
	const unsigned char *x=(const unsigned char*)p;
	return x[0];
}

unsigned long long int read64(const void *p){
	const unsigned char *x=(const unsigned char*)p;
	return x[0]|(x[1]<<8)|(x[2]<<16)|(x[3]<<24)|( (unsigned long long int)(x[4]|(x[5]<<8)|(x[6]<<16)|(x[7]<<24)) <<32);
}

void write32(void *p, const unsigned int n){
	unsigned char *x=(unsigned char*)p;
	x[0]=n&0xff,x[1]=(n>>8)&0xff,x[2]=(n>>16)&0xff,x[3]=(n>>24)&0xff;
}

void write24(void *p, const unsigned int n){
	unsigned char *x=(unsigned char*)p;
	x[0]=n&0xff,x[1]=(n>>8)&0xff,x[2]=(n>>16)&0xff;
}

void write16(void *p, const unsigned short n){
	unsigned char *x=(unsigned char*)p;
	x[0]=n&0xff,x[1]=(n>>8)&0xff;
}

void write8(void *p, const unsigned char n){
	unsigned char *x=(unsigned char*)p;
	x[0]=n&0xff;
}

void write64(void *p, const unsigned long long int n){
	unsigned char *x=(unsigned char*)p;
	x[0]=n&0xff,x[1]=(n>>8)&0xff,x[2]=(n>>16)&0xff,x[3]=(n>>24)&0xff,
	x[4]=(n>>32)&0xff,x[5]=(n>>40)&0xff,x[6]=(n>>48)&0xff,x[7]=(n>>56)&0xff;
}

void die(){
	sceKernelExitGame();while(1);
}

char* myfgets(char *buf,int n,FILE *fp){ //accepts LF/CRLF
	char *ret=fgets(buf,n,fp);
	if(!ret)return NULL;
	if(strlen(buf)&&buf[strlen(buf)-1]=='\n')buf[strlen(buf)-1]=0;
	if(strlen(buf)&&buf[strlen(buf)-1]=='\r')buf[strlen(buf)-1]=0;
	return ret;
}
/*
void rm_rf(char *target){ // last byte has to be '/' *** target itself won't be erased
}
*/

void mkpath(char *path){ // last byte has to be '/' (after final '/' will be ignored)
	int i=2;
	char dir[768];
	if(!path||!*path||!path[1])return;
	for(memset(dir,0,768);i<strlen(path);i++)
		if(path[i]=='/')strncpy(dir,path,i),sceIoMkdir(dir,0777),i++;
}

int filelength(int fd){
	struct stat st;
	if(fstat(fd,&st))return 0;
	return st.st_size;
}

int sceIoGetFileLength(int fd){
	long long int offset,ret;
	offset = sceIoLseek(fd, 0, PSP_SEEK_CUR);
	ret = sceIoLseek(fd, 0, PSP_SEEK_END);
	sceIoLseek(fd, offset, PSP_SEEK_SET);
	return ret;
}

int copy(const char *old, const char *new){
	FILE *in=fopen(old,"rb");
	if(!in)return 1;
	FILE *out=fopen(new,"wb");
	if(!out){fclose(in);return 2;}
	//int size=filelength(fileno(in));
	int read,cur=0;
	//
	while((read=fread(libpuer_buf,1,BUFLEN,in))>0){
		cur+=read;
		fwrite(libpuer_buf,1,read,out);
		//printf("%s %8d / %8d\r","Copying",cur,size);
	}
	//
	fclose(out);
	fclose(in);
	return 0;
}

char *strcpy_safe(char *s1, const char *s2){
	if(!s1||!s2)return NULL;
	return strcpy(s1,s2);
}

char *getextname(char *s){
	int i;
	if(!s)return NULL;
	if(!*s)return "";
	for(i=strlen(s)-1;i>0;i--){
		if(s[i]=='/'){i++;break;}
		if(s[i]=='.')break;
	}
	return s+i;
}

char *getfilename(char *s){
	int i;
	if(!s)return NULL;
	if(!*s)return "";
	for(i=strlen(s);i>0;i--){
		if(s[i-1]=='/'){break;}
		//if(s[i]=='.')break;
	}
	return s+i;
}

#define MAX_HOUR 23
#define MAX_MINUTE 59
#define MAX_SECOND 60

#define MAX_MONTH 11
#define MIN_MONTH 0
#define MAX_DAY 31
#define MIN_DAY 1

int UTCToDateTime(time_t epochTime, u16 *date, u16 *time){
	struct tm timeParts;

	localtime_r(&epochTime, &timeParts);

	if(validateTM(&timeParts))return -1;
	
	if(date)*date=
		(((timeParts.tm_year - 80) & 0x7F) <<9) |	// Adjust for MS-FAT base year (1980 vs 1900 for tm_year)
		(((timeParts.tm_mon + 1) & 0xF) << 5) |
		(timeParts.tm_mday & 0x1F);

	if(time)*time=
		((timeParts.tm_hour & 0x1F) << 11) |
		((timeParts.tm_min & 0x3F) << 5) |
		((timeParts.tm_sec >> 1) & 0x1F);

	return 0;
}

int validateTM(struct tm *timeParts){
	if(!timeParts)return -1;
	if ((timeParts->tm_mon < MIN_MONTH) || (timeParts->tm_mon > MAX_MONTH)) return -1;
	if ((timeParts->tm_mday < MIN_DAY) || (timeParts->tm_mday > MAX_DAY)) return -1;
	if ((timeParts->tm_hour < 0) || (timeParts->tm_hour > MAX_HOUR))	return -1;
	if ((timeParts->tm_min < 0) || (timeParts->tm_min > MAX_MINUTE)) return -1;
	if ((timeParts->tm_sec < 0) || (timeParts->tm_sec > MAX_SECOND)) return -1;
	return 0;
}

int fexists(const char *path){
	struct stat st;
	if(stat(path,&st))return 0;
	return (st.st_mode&S_IFDIR)?2:1;
}

int strchrindex(const char *s, const int c, const int idx){
	const char *ret=strchr(s+idx,c);
	if(!ret)return -1;
	return ret-s;
}

int strstrindex(const char *s, const char *c, const int idx){
	const char *ret=strstr(s+idx,c);
	if(!ret)return -1;
	return ret-s;
}

void changefileext(char *fn, const char *ext){
	if(!fn||!ext||!*ext)return;
	strcpy(getextname(fn),ext);
}
