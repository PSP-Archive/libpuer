#ifndef LIBPUER_H
#define LIBPUER_H

/*
	libpuer - a fundamental framework for PSP development
	Special Thanks: H. Sonoda
	Special Thanks: M. Nezaki (@Nzaki0716)

	*** Portion Notice ***
	minIni (C) ITB CompuPhase under Apache License 2.0 (BSD like, compatible with GPLv3+)

	[Individual]
	HMAC-SHA1 code (C) Internet Society
	zenkaku library [removed due to UNINTENDED articles]

	You may use libpuer in both open source and proprietary software without restriction.
	libpuer is distributed AS-IS without WARRANTY.
*/

#define VERSION "r3d3.130422"
#define DATE __DATE__" "__TIME__" GMT+09:00"
#define ENV "devkitPSP r16"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h> //validateTM, struct tm
#include <sys/stat.h>

#include "include/psp.h"
#include "include/kernelsdk.h"

#include "include/minIni.h"
#include "include/sha1.h"

#define DISC_EBOOT "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN"

#ifdef DEBUG
	#define debug printf
	#define debug_waitpsplink() sceKernelDelayThread(1500000)
#else
	#define debug(...)
	#define debug_waitpsplink()
#endif

#ifndef __cplusplus
#include <stdbool.h>
#else
extern "C" {
#endif

/* definition and struct */

#define attrinline   static __attribute__((always_inline))
#define attrnoinline __attribute__((noinline))
#define attrnoreturn __attribute__((noreturn))

#define BIT0 (1<<0)
#define BIT1 (1<<1)
#define BIT2 (1<<2)
#define BIT3 (1<<3)
#define BIT4 (1<<4)
#define BIT5 (1<<5)
#define BIT6 (1<<6)
#define BIT7 (1<<7)
#define BIT8 (1<<8)
#define BIT9 (1<<9)
#define BIT10 (1<<10)
#define BIT11 (1<<11)
#define BIT12 (1<<12)
#define BIT13 (1<<13)
#define BIT14 (1<<14)
#define BIT15 (1<<15)

#define align2(i) (((i)+1)&~1)
#define align4(i) (((i)+3)&~3)
#define align8(i) (((i)+7)&~7)
#define align256(i) (((i)+255)&~255)
#define align512(i) (((i)+511)&~511)

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#define mainThreadParam(name,priority,stackKB,attribute) \
	PSP_MAIN_THREAD_NAME(name); \
	PSP_MAIN_THREAD_PRIORITY(priority); \
	PSP_MAIN_THREAD_STACK_SIZE_KB(stackKB); \
	PSP_MAIN_THREAD_ATTR(attribute); \

#define arraysize(a) (sizeof(a)/sizeof(*a))

#define PSP_FIRMWARE(f) ((((f >> 8) & 0xF) << 24) | (((f >> 4) & 0xF) << 16) | ((f & 0xF) << 8) | 0x10)
#define KERNELVERSION (sceKernelDevkitVersion()>>8)

/* hook */
// func physical address > 0x03ffffff && func op isn't "syscall 0x15"
//#define isLinked(func) ((((u32)func)&0x0fffffff)>0x03ffffff&&*(u32*)func!=0x0000054c)
#define isLinked(func) ((((u32)func)&0x0fffffff)>0x03ffffff&&read32(func)!=0x0000054c)

#define JAL_OPCODE	0x0C000000
#define J_OPCODE	0x08000000
#define SC_OPCODE	0x0000000C
#define JR_RA		0x03e00008
#define NOP			0x00000000

#define MAKE_JUMP(a, f)			_sw(J_OPCODE | (((u32)(f) & 0x0ffffffc) >> 2), a)
#define MAKE_CALL(a, f)			_sw(JAL_OPCODE | (((u32)(f) >> 2)  & 0x03ffffff), a)
#define MAKE_CALL_FUNC(f)		(JAL_OPCODE | (((u32)(f) >> 2)  & 0x03ffffff))
#define MAKE_SYSCALL(a, n)		_sw(SC_OPCODE | (n << 6), a)
#define JUMP_TARGET(x)			(J_OPCODE | ((x & 0x03FFFFFF) << 2))
#define MAKE_DUMMY_FUNCTION0(a)	_sw(JR_RA, a),_sw(0x00001021, a+4)
#define MAKE_DUMMY_FUNCTION1(a)	_sw(JR_RA, a),_sw(0x24020001, a+4)
#define REDIRECT_FUNCTION(a, f)	_sw(J_OPCODE | (((u32)(f) >> 2)  & 0x03ffffff), a),_sw(NOP, a+4)

/* stub */
typedef void* (*TsctrlSetStartModuleExtra)(int(*)());
extern TsctrlSetStartModuleExtra PsctrlSetStartModuleExtra;
typedef void (*TsctrlSESetUmdFileEx)(char*,void*);
extern TsctrlSESetUmdFileEx PsctrlSESetUmdFileEx;
typedef int (*Tversionspoofer)(u8*,u32,u32*);
extern Tversionspoofer Pversionspoofer;
typedef int (*TvctrlVSHRegisterVshMenu)(int(*)(SceCtrlData*,int));
extern TvctrlVSHRegisterVshMenu PvctrlVSHRegisterVshMenu;
typedef char* (*TsceKernelGetGameInfo)();
extern TsceKernelGetGameInfo PsceKernelGetGameInfo;

#define CFW_PRO   (Pversionspoofer)
#define CFW_TN    (!Pversionspoofer&&PvctrlVSHRegisterVshMenu)
#define CFW_ME    (!PvctrlVSHRegisterVshMenu&&PsctrlSetStartModuleExtra)
#define CFW_OTHER (!PvctrlVSHRegisterVshMenu&&!PsctrlSetStartModuleExtra)

/* libpuer API */
#define BUFLEN 65536
extern u8 libpuer_buf[BUFLEN];
#define libpuer_cbuf ((char*)libpuer_buf)
extern char mypath[768], *myfile;
extern int argc;
extern char *argv[]; //not **argv
extern int running;
extern int PSP_CTRL_OK,PSP_CTRL_CANCEL;

int executeEBOOT(const char* path);
int executeUpdater(const char* path);
int executeUMD();
int executeISO(const char *path, int mode);
int executeAny(const char* path);
const char *get_eboot_id(const char *eboot_name);
int ResolveDyn();

u32 toNid(const char *name);

//hook
void* search_module_export(SceModule2 *pMod, const char *szLib, u32 nid);
void* search_module_stub(SceModule2 *pMod, const char *szLib, u32 nid);

//memstream
typedef struct{
	u8  *p;
	u32 current;
	u32 size;
} memstream;

memstream *mopen(void *p, const u32 size, memstream *s);
int mclose(memstream *s);
int mgetc(memstream *s);
int mputc(const int c, memstream *s);
int mrewind(memstream *s);
int mavail(memstream *s);
int mtell(memstream *s);
int mlength(memstream *s);
int mread(void *buf, const u32 size, memstream *s);
int mwrite(void *buf, const u32 size, memstream *s);
int mcopy(memstream *to, const u32 size, memstream *s);
int mseek(memstream *s, const int offset, const int whence);
unsigned int mread32(memstream *s);
unsigned short mread16(memstream *s);
unsigned char mread8(memstream *s);
int mwrite32(const unsigned int n, memstream *s);
int mwrite16(const unsigned short n, memstream *s);
int mwrite8(const unsigned char n, memstream *s);

// util
int SetupExitCallback();
void ClearCaches();

u32 getNid(u32 HASH,u32 FW500,u32 FW620,u32 FW635,u32 FW660);

attrnoreturn void die();
char* myfgets(char *buf,int n,FILE *fp);
//void rm_rf(char *target);
void mkpath(char *path);

//normal version (1 byte i/o)
unsigned int read32(const void *p);
unsigned int read24(const void *p);
unsigned short read16(const void *p);
unsigned char read8(const void *p);
unsigned long long int read64(const void *p);
void write32(void *p, const unsigned int n);
void write24(void *p, const unsigned int n);
void write16(void *p, const unsigned short n);
void write8(void *p, const unsigned char n);
void write64(void *p, const unsigned long long int n);

//direct version (4 bytes i/o) *** cannot be used on big-endian machine
unsigned int readAddr(void *mem);
unsigned int readAddr24(void *mem);
unsigned short readAddr16(void *mem);
unsigned char readAddr8(void *mem);
unsigned long long int readAddr64(void *mem);
void writeAddr(void *mem, const unsigned int value);
//void writeAddr24(void *mem, const unsigned int value);
void writeAddr16(void *mem, const unsigned short value);
void writeAddr8(void *mem, const unsigned char value);
void writeAddr64(void *mem, const unsigned long long int value);

int filelength(int fd);
int sceIoGetFileLength(int fd);
int copy(const char *old, const char *_new);
char *strcpy_safe(char *s1, const char *s2);
char *getextname(char *s);
char *getfilename(char *s);
void changefileext(char *fn, const char *ext);

int UTCToDateTime(time_t epochTime, u16 *date, u16 *time);
int validateTM(struct tm *timeParts);
int fexists(const char *path);
int strchrindex(const char *s, const int c, const int idx);
int strstrindex(const char *s, const char *c, const int idx);

#ifdef __cplusplus
}
#endif
#endif //included
