// PSAR dumper for Updater data 
// Original author: PspPet
//
// Contributions:
//
// Vampire (bugfixes)
// Nem (ipl decryption)
// Dark_AleX (2.60-2.80 decryption)
// Noobz (3.00-3.02 decryption)
// Team C+D (3.03-3.52 decryption)
// M33 Team (3.60-3.71 decryption) + recode for 2.XX+ kernels 
// bbtgp (6.00-6.20 decryption)

#include "../../libpuer/libpuer.h"

#include "libpsardumper.h"
#include "pspdecrypt.h"

PSP_MODULE_INFO("PopsDeco_rev1", PSP_MODULE_USER, 1, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);

#define printf pspDebugScreenPrintf

////////////////////////////////////////////////////////////////////
// big buffers for data. Some system calls require 64 byte alignment

// big enough for the full PSAR file

#define PSAR_BUFFER_SIZE	9400000

static u8 g_dataPSAR[PSAR_BUFFER_SIZE] __attribute__((aligned(64))); 

// big enough for the largest (multiple uses)
static u8 g_dataOut[3000000] __attribute__((aligned(0x40)));
   
// for deflate output
//u8 g_dataOut2[3000000] __attribute__((aligned(0x40)));
static u8 *g_dataOut2;   

void ErrorExit(int milisecs, char *fmt, ...)
{
	va_list list;
	char msg[256];	

	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);

	printf(msg);
	
	//sceKernelDelayThread(milisecs*1000);
	
	printf("\n\nPress X to exit\n");
	
	while (1)
	{
		SceCtrlData pad;
		sceCtrlReadBufferPositive(&pad, 1);
		if (pad.Buttons & PSP_CTRL_CROSS)
			break;
		sceKernelDelayThread(10000);
	}

	
	sceKernelExitGame();
}

////////////////////////////////////////////////////////////////////
// File helpers

int ReadFile(char *file, int seek, void *buf, int size)
{
	SceUID fd = sceIoOpen(file, PSP_O_RDONLY, 0);
	if (fd < 0)
		return fd;

	if (seek > 0)
	{
		if (sceIoLseek(fd, seek, PSP_SEEK_SET) != seek)
		{
			sceIoClose(fd);
			return -1;
		}
	}

	int read = sceIoRead(fd, buf, size);
	
	sceIoClose(fd);
	return read;
}

int WriteFile(char *file, void *buf, int size)
{
	SceUID fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	
	if (fd < 0)
	{
		return fd;
	}

	int written = sceIoWrite(fd, buf, size);

	sceIoClose(fd);
	return written;
}

static char com_table[0x4000];
static int comtable_size;

static char _1g_table[0x4000];
static int _1gtable_size;

static char _2g_table[0x4000];
static int _2gtable_size;

static char _3g_table[0x4000];
static int _3gtable_size;

static char _4g_table[0x4000];
static int _4gtable_size;

static char _5g_table[0x4000];
static int _5gtable_size;

static char _6g_table[0x4000];
static int _6gtable_size;

static char _7g_table[0x4000];
static int _7gtable_size;

static char _8g_table[0x4000];
static int _8gtable_size;

static char _9g_table[0x4000];
static int _9gtable_size;

static char _10g_table[0x4000];
static int _10gtable_size;

static char _11g_table[0x4000];
static int _11gtable_size;

static char _12g_table[0x4000];
static int _12gtable_size;

enum
{
	MODE_ENCRYPT_SIGCHECK,
	MODE_ENCRYPT,
	MODE_DECRYPT,
};

static int FindTablePath(char *table, int table_size, char *number, char *szOut)
{
	int i, j, k;

	for (i = 0; i < table_size-5; i++)
	{
		if (strncmp(number, table+i, 5) == 0)
		{
			for (j = 0, k = 0; ; j++, k++)
			{
				if (table[i+j+6] < 0x20)
				{
					szOut[k] = 0;
					break;
				}

				if (table[i+5] == '|' && !strncmp(table+i+6, "flash", 5) &&
					j == 6)
				{
					szOut[6] = ':';
					szOut[7] = '/';
					k++;
				}
				else if (table[i+5] == '|' && !strncmp(table+i+6, "ipl", 3) &&
					j == 3)
				{
					szOut[3] = ':';
					szOut[4] = '/';
					k++;
				}
				else
				{				
					szOut[k] = table[i+j+6];
				}
			}

			return 1;
		}
	}

	return 0;
}

/*
static int FindReboot(u8 *input, u8 *output, int size)
{
	int i;

	for (i = 0; i < (size - 0x30); i++)
	{
		if (memcmp(input+i, "~PSP", 4) == 0)
		{
			size = *(u32 *)&input[i+0x2C];

			memcpy(output, input+i, size);
			return size;
		}
	}

	return -1;
}

static void ExtractReboot(int mode, char *loadexec, char *reboot, char *rebootname)
{
	int s = ReadFile(loadexec, 0, g_dataOut, sizeof(g_dataOut));

	if (s <= 0)
		return;
	
	printf("Extracting %s... ", rebootname);

	if (mode != MODE_DECRYPT)
	{
		if (mode == MODE_ENCRYPT_SIGCHECK)
		{
			memcpy(g_dataOut2, g_dataOut, s);
			pspSignCheck(g_dataOut2);

			if (WriteFile(loadexec, g_dataOut2, s) != s)
			{
				ErrorExit(5000, "Cannot write %s.\n", loadexec);
			}
		}
			
		s = pspDecryptPRX(g_dataOut, g_dataOut2, s);
		if (s <= 0)
		{
			ErrorExit(5000, "Cannot decrypt %s.\n", loadexec);
		}

		s = pspDecompress(g_dataOut2, g_dataOut, sizeof(g_dataOut));
		if (s <= 0)
		{
			ErrorExit(5000, "Cannot decompress %s.\n", loadexec);
		}
	}

	s = FindReboot(g_dataOut, g_dataOut2, s);
	if (s <= 0)
	{
		ErrorExit(5000, "Cannot find %s inside loadexec.\n", rebootname);
	}

	s = pspDecryptPRX(g_dataOut2, g_dataOut, s);
	if (s <= 0)
	{
		ErrorExit(5000, "Cannot decrypt %s.\n", rebootname);
	}

	WriteFile(reboot, g_dataOut, s);

	s = pspDecompress(g_dataOut, g_dataOut2, sizeof(g_dataOut));
	if (s <= 0)
	{
		ErrorExit(5000, "Cannot decompress %s (0x%08X).\n", rebootname, s);
	}

	if (WriteFile(reboot, g_dataOut2, s) != s)
	{
		ErrorExit(5000, "Cannot write %s.\n", reboot);
	}

	printf("done.\n");
}
*/

static char *GetVersion(char *buf)
{
	char *p = strrchr(buf, ',');

	if (!p)
		return NULL;

	return p+1;
}

static int is5Dnum(char *str)
{
	int len = strlen(str);

	if (len != 5)
		return 0;

	int i;

	for (i = 0; i < len; i++)
	{
		if (str[i] < '0' || str[i] > '9')
			return 0;
	}

	return 1;
}

int LoadStartModule(char *module, int partition)
{
	SceUID mod = sceKernelLoadModule(module, 0, NULL);

	if (mod < 0)
		return mod;

	return sceKernelStartModule(mod, 0, NULL, NULL, NULL);
}

#include "expand.h"

extern void check(const char *md5sum_name);
#define argc(a) (sizeof(a)/sizeof(*(a))/2)
int main(){
    int s;//, mode=0, res;
	//u8 pbp_header[0x28];
	//SceUID fd;
	//int error = 0;
	//int psar_pos = 0, psar_offs;
	//int table_mode;

	sceIoChdir(mypath);
	pspDebugScreenInit();

	printf("PopsDeco 0.06a.111111 based on PSAR Dumper by PspPet\n");
	printf("IPL Decryption By Nem.\n");
	printf("2.60 - 2.80 Decryption By Dark_AleX.\n");
	printf("3.00 - 3.30 Decryption By Team Noobz.\n");
	printf("3.03 - 3.52 Decryption By Team C+D.\n");
	printf("3.60 + 3.71 + 5.00 Decryption + Recode for 2.xx+ Kernels By M33 Team.\n");
	printf("3.73 + 3.80 + 3.90 + 3.93 + 3.95 + 4.05 Decryption By RedSquirrel.\n");
	printf("4.00 Decryption By noRTU.\n");
	printf("4.01 Decryption By PSPGen.\n");
	printf("5.50 User Mode By Red Bull PSP Team.\n");
	printf("5.55 + 6.00 User Mode By Yoshihiro.\n");
	//printf("6.10 User Mode By Ultradog.\n");
	printf("6.00-6.20 Decryption by bbtgp.\n");
	printf("\n");
	
	int can_extract=0;
	SceUID mod;
	if (KERNELVERSION < 0x020701){
		printf("Dumping OFW requires CFW 2.71 or higher.\n");
	}else if((mod = LoadStartModule("libpsardumper.prx", PSP_MEMORY_PARTITION_KERNEL))<0){
		printf("Error 0x%08X loading/starting libpsardumper.prx.\n", mod);
	//}else if((mod = LoadStartModule("pspdecrypt.prx", PSP_MEMORY_PARTITION_KERNEL))<0){
	//	printf("Error 0x%08X loading/starting pspdecrypt.prx.\n", mod);
	}else{can_extract=1;}

	printf("Press triangle to check pops modules.\n");
	if(can_extract)printf("Press cross to start dumping pops modules.\n");
	printf("Press circle to abort.\n");
	printf("\n");

	for(;;){
		SceCtrlData pad;
		sceCtrlReadBufferPositive(&pad, 1);

		if(can_extract && (pad.Buttons & PSP_CTRL_CROSS)){
			break;
		}else if(pad.Buttons & PSP_CTRL_CIRCLE){
			sceKernelExitGame();
		}else if(pad.Buttons & PSP_CTRL_TRIANGLE){
			check("popsloader.md5sum");
			printf("Auto exit in 3 seconds.\n");
			sceKernelDelayThread(3000000);
			sceKernelExitGame();
		}

		sceKernelDelayThread(1000);
	}
	sceIoMkdir("ms0:/seplugins", 0777);
	sceIoMkdir("ms0:/seplugins/popsloader", 0777);

	sceKernelVolatileMemLock(0, (void *)&g_dataOut2, &s);

/*
	ExtractReboot(mode, "ms0:/F0/kd/loadexec.prx", "ms0:/F0/reboot.bin", "reboot.bin");
	ExtractReboot(mode, "ms0:/F0/kd/loadexec_01g.prx", "ms0:/F0/reboot.bin", "reboot.bin");
	ExtractReboot(mode, "ms0:/F0/kd/loadexec_02g.prx", "ms0:/F0/reboot_02g.bin", "reboot_02g.bin");
	ExtractReboot(mode, "ms0:/F0/kd/loadexec_03g.prx", "ms0:/F0/reboot_03g.bin", "reboot_03g.bin");
	ExtractReboot(mode, "ms0:/F0/kd/loadexec_04g.prx", "ms0:/F0/reboot_04g.bin", "reboot_04g.bin");
	ExtractReboot(mode, "ms0:/F0/kd/loadexec_05g.prx", "ms0:/F0/reboot_05g.bin", "reboot_05g.bin");
	ExtractReboot(mode, "ms0:/F0/kd/loadexec_06g.prx", "ms0:/F0/reboot_06g.bin", "reboot_06g.bin");
	ExtractReboot(mode, "ms0:/F0/kd/loadexec_07g.prx", "ms0:/F0/reboot_07g.bin", "reboot_07g.bin");
	ExtractReboot(mode, "ms0:/F0/kd/loadexec_08g.prx", "ms0:/F0/reboot_08g.bin", "reboot_08g.bin");
	ExtractReboot(mode, "ms0:/F0/kd/loadexec_09g.prx", "ms0:/F0/reboot_09g.bin", "reboot_09g.bin");
*/

	{	char *p401[]={
			"ms0:/F0/kd/impose.prx","ms0:/seplugins/popsloader/impose.prx",
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/popsman401.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/pops401.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/pafmini401.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/libpspvmc401.prx",
		}; Expand("ms0:/401.PBP",argc(p401),p401);}
	{	char *p300[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/popsman300.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/pops300.prx",
			"ms0:/F0/kd/meaudio.prx","ms0:/seplugins/popsloader/meaudio300.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/pafmini300.prx",
		}; Expand("ms0:/300.PBP",argc(p300),p300);}
	{	char *p301[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/popsman301.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/pops301.prx",
			"ms0:/F0/kd/meaudio.prx","ms0:/seplugins/popsloader/meaudio301.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/pafmini301.prx",
		}; Expand("ms0:/301.PBP",argc(p301),p301);}
	{	char *p302[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/popsman302.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/pops302.prx",
			"ms0:/F0/kd/meaudio.prx","ms0:/seplugins/popsloader/meaudio302.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/pafmini302.prx",
		}; Expand("ms0:/302.PBP",argc(p302),p302);}
	{	char *p303[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/popsman303.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/pops303.prx",
			"ms0:/F0/kd/meaudio.prx","ms0:/seplugins/popsloader/meaudio303.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/pafmini303.prx",
		}; Expand("ms0:/303.PBP",argc(p303),p303);}
	{	char *p310[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/popsman310.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/pops310.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/pafmini310.prx",
		}; Expand("ms0:/310.PBP",argc(p310),p310);}
	{	char *p311[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/popsman311.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/pops311.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/pafmini311.prx",
		}; Expand("ms0:/311.PBP",argc(p311),p311);}
	{	char *p330[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/popsman330.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/pops330.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/pafmini330.prx",
		}; Expand("ms0:/330.PBP",argc(p330),p330);}
	{	char *p340[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/popsman340.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/pops340.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/pafmini340.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/libpspvmc340.prx",
		}; Expand("ms0:/340.PBP",argc(p340),p340);}
	{	char *p351[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/popsman351.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/pops351.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/pafmini351.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/libpspvmc351.prx",
		}; Expand("ms0:/351.PBP",argc(p351),p351);}
	{	char *p352[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/popsman352.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/pops352.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/pafmini352.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/libpspvmc352.prx",
		}; Expand("ms0:/352.PBP",argc(p352),p352);}
	{	char *p371[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/popsman371.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/pops371.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/pafmini371.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/libpspvmc371.prx",
		}; Expand("ms0:/371.PBP",argc(p371),p371);}
	{	char *p372[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/popsman372.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/pops372.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/pafmini372.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/libpspvmc372.prx",
		}; Expand("ms0:/372.PBP",argc(p372),p372);}
	{	char *p380[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/popsman380.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/pops380.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/pafmini380.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/libpspvmc380.prx",
		}; Expand("ms0:/380.PBP",argc(p380),p380);}
	{	char *p390[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/popsman390.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/pops390.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/pafmini390.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/libpspvmc390.prx",
		}; Expand("ms0:/390.PBP",argc(p390),p390);}

	//Add what you want.

	scePowerTick(0);
	//ErrorExit(10000, "Done.\n");
	printf("Auto exit in 3 seconds.\n");
	sceKernelDelayThread(3000000);
	sceKernelExitGame();

    return 0;
}

