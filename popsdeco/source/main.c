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

PSP_MODULE_INFO("PopsDeco_rev2", PSP_MODULE_USER, 1, 1);
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

	printf("PopsDeco 0.15.150124 based on PSAR Dumper by PspPet\n");
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
	sceIoMkdir("ms0:/seplugins/popsloader/modules", 0777);

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
			"ms0:/F0/kd/impose.prx","ms0:/seplugins/popsloader/modules/impose.prx",
			"ms0:/F0/kd/resource/impose.rsc","ms0:/seplugins/popsloader/modules/impose.rsc",
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/401/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/401/pops.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/401/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/401/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/401/libpspvmc.prx",
		}; Expand("ms0:/401.PBP","ms0:/seplugins/popsloader/modules/401",argc(p401),p401);}
/*
	{	char *p300[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/300/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/300/pops.prx",
			"ms0:/F0/kd/meaudio.prx","ms0:/seplugins/popsloader/modules/300/meaudio.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/300/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/300/pafmini.prx",
		}; Expand("ms0:/300.PBP","ms0:/seplugins/popsloader/modules/300",argc(p300),p300);}
	{	char *p301[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/301/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/301/pops.prx",
			"ms0:/F0/kd/meaudio.prx","ms0:/seplugins/popsloader/modules/301/meaudio.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/301/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/301/pafmini.prx",
		}; Expand("ms0:/301.PBP","ms0:/seplugins/popsloader/modules/301",argc(p301),p301);}
*/
	{	char *p302[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/302/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/302/pops.prx",
			"ms0:/F0/kd/meaudio.prx","ms0:/seplugins/popsloader/modules/302/meaudio.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/302/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/302/pafmini.prx",
		}; Expand("ms0:/302.PBP","ms0:/seplugins/popsloader/modules/302",argc(p302),p302);}
	{	char *p303[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/303/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/303/pops.prx",
			"ms0:/F0/kd/meaudio.prx","ms0:/seplugins/popsloader/modules/303/meaudio.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/303/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/303/pafmini.prx",
		}; Expand("ms0:/303.PBP","ms0:/seplugins/popsloader/modules/303",argc(p303),p303);}
	{	char *p310[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/310/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/310/pops.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/310/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/310/pafmini.prx",
		}; Expand("ms0:/310.PBP","ms0:/seplugins/popsloader/modules/310",argc(p310),p310);}
	{	char *p311[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/311/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/311/pops.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/311/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/311/pafmini.prx",
		}; Expand("ms0:/311.PBP","ms0:/seplugins/popsloader/modules/311",argc(p311),p311);}
	{	char *p330[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/330/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/330/pops.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/330/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/330/pafmini.prx",
		}; Expand("ms0:/330.PBP","ms0:/seplugins/popsloader/modules/330",argc(p330),p330);}
	{	char *p340[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/340/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/340/pops.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/340/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/340/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/340/libpspvmc.prx",
		}; Expand("ms0:/340.PBP","ms0:/seplugins/popsloader/modules/340",argc(p340),p340);}
	{	char *p350[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/350/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/350/pops.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/350/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/350/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/350/libpspvmc.prx",
		}; Expand("ms0:/350.PBP","ms0:/seplugins/popsloader/modules/350",argc(p350),p350);}
	{	char *p351[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/351/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/351/pops.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/351/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/351/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/351/libpspvmc.prx",
		}; Expand("ms0:/351.PBP","ms0:/seplugins/popsloader/modules/351",argc(p351),p351);}
	{	char *p352[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/352/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/352/pops.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/352/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/352/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/352/libpspvmc.prx",
		}; Expand("ms0:/352.PBP","ms0:/seplugins/popsloader/modules/352",argc(p352),p352);}
	{	char *p371[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/371/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/371/pops.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/371/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/371/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/371/libpspvmc.prx",
		}; Expand("ms0:/371.PBP","ms0:/seplugins/popsloader/modules/371",argc(p371),p371);}
	{	char *p372[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/372/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/372/pops.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/372/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/372/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/372/libpspvmc.prx",
		}; Expand("ms0:/372.PBP","ms0:/seplugins/popsloader/modules/372",argc(p372),p372);}
	{	char *p380[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/380/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/380/pops.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/380/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/380/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/380/libpspvmc.prx",
		}; Expand("ms0:/380.PBP","ms0:/seplugins/popsloader/modules/380",argc(p380),p380);}
	{	char *p390[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/390/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/390/pops.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/390/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/390/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/390/libpspvmc.prx",
		}; Expand("ms0:/390.PBP","ms0:/seplugins/popsloader/modules/390",argc(p390),p390);}
	{	char *p393[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/393/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/393/pops.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/393/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/393/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/393/libpspvmc.prx",
		}; Expand("ms0:/393.PBP","ms0:/seplugins/popsloader/modules/393",argc(p393),p393);}
	{	char *p396[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/396/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/396/pops.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/396/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/396/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/396/libpspvmc.prx",
		}; Expand("ms0:/396.PBP","ms0:/seplugins/popsloader/modules/396",argc(p396),p396);}
	{	char *p500[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/500/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/500/pops.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/500/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/500/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/500/libpspvmc.prx",
		}; Expand("ms0:/500.PBP","ms0:/seplugins/popsloader/modules/500",argc(p500),p500);}
	{	char *p501[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/501/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/501/pops.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/501/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/501/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/501/libpspvmc.prx",
		}; Expand("ms0:/501.PBP","ms0:/seplugins/popsloader/modules/501",argc(p501),p501);}
	{	char *p503[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/503/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/503/pops.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/503/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/503/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/503/libpspvmc.prx",
		}; Expand("ms0:/503.PBP","ms0:/seplugins/popsloader/modules/503",argc(p503),p503);}
/*
	{	char *p505[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/505/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/505/pops.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/505/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/505/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/505/libpspvmc.prx",
		}; Expand("ms0:/505.PBP","ms0:/seplugins/popsloader/modules/505",argc(p505),p505);}
*/
	{	char *p550[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/550/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/550/pops.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/550/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/550/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/550/libpspvmc.prx",
		}; Expand("ms0:/550.PBP","ms0:/seplugins/popsloader/modules/550",argc(p550),p550);}
	{	char *p551[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/551/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/551/pops.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/551/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/551/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/551/libpspvmc.prx",
		}; Expand("ms0:/551.PBP","ms0:/seplugins/popsloader/modules/551",argc(p551),p551);}
#if 0
	{	char *p555[]={
			//"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/555/popsman.prx", //cannot decrypt
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/555/pops.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/555/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/555/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/555/libpspvmc.prx",
		}; Expand("ms0:/555.PBP","ms0:/seplugins/popsloader/modules/555",argc(p555),p555);}
#endif
	{	char *p600[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/600/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/600/pops.prx",
			"ms0:/F0/kd/pops_01g.prx","ms0:/seplugins/popsloader/modules/600/pops_01g.prx",
			"ms0:/F0/kd/pops_02g.prx","ms0:/seplugins/popsloader/modules/600/pops_02g.prx",
			"ms0:/F0/kd/pops_03g.prx","ms0:/seplugins/popsloader/modules/600/pops_03g.prx",
			"ms0:/F0/kd/pops_04g.prx","ms0:/seplugins/popsloader/modules/600/pops_04g.prx",
			"ms0:/F0/kd/pops_05g.prx","ms0:/seplugins/popsloader/modules/600/pops_05g.prx",
			"ms0:/F0/kd/pops_06g.prx","ms0:/seplugins/popsloader/modules/600/pops_06g.prx",
			"ms0:/F0/kd/pops_07g.prx","ms0:/seplugins/popsloader/modules/600/pops_07g.prx",
			"ms0:/F0/kd/pops_08g.prx","ms0:/seplugins/popsloader/modules/600/pops_08g.prx",
			"ms0:/F0/kd/pops_09g.prx","ms0:/seplugins/popsloader/modules/600/pops_09g.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/600/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/600/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/600/libpspvmc.prx",
		}; Expand("ms0:/600.PBP","ms0:/seplugins/popsloader/modules/600",argc(p600),p600);}
	{	char *p610[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/610/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/610/pops.prx",
			"ms0:/F0/kd/pops_01g.prx","ms0:/seplugins/popsloader/modules/610/pops_01g.prx",
			"ms0:/F0/kd/pops_02g.prx","ms0:/seplugins/popsloader/modules/610/pops_02g.prx",
			"ms0:/F0/kd/pops_03g.prx","ms0:/seplugins/popsloader/modules/610/pops_03g.prx",
			"ms0:/F0/kd/pops_04g.prx","ms0:/seplugins/popsloader/modules/610/pops_04g.prx",
			"ms0:/F0/kd/pops_05g.prx","ms0:/seplugins/popsloader/modules/610/pops_05g.prx",
			"ms0:/F0/kd/pops_06g.prx","ms0:/seplugins/popsloader/modules/610/pops_06g.prx",
			"ms0:/F0/kd/pops_07g.prx","ms0:/seplugins/popsloader/modules/610/pops_07g.prx",
			"ms0:/F0/kd/pops_08g.prx","ms0:/seplugins/popsloader/modules/610/pops_08g.prx",
			"ms0:/F0/kd/pops_09g.prx","ms0:/seplugins/popsloader/modules/610/pops_09g.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/610/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/610/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/610/libpspvmc.prx",
		}; Expand("ms0:/610.PBP","ms0:/seplugins/popsloader/modules/610",argc(p610),p610);}
	{	char *p610go[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/610/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/610/pops.prx",
			"ms0:/F0/kd/pops_01g.prx","ms0:/seplugins/popsloader/modules/610/pops_01g.prx",
			"ms0:/F0/kd/pops_02g.prx","ms0:/seplugins/popsloader/modules/610/pops_02g.prx",
			"ms0:/F0/kd/pops_03g.prx","ms0:/seplugins/popsloader/modules/610/pops_03g.prx",
			"ms0:/F0/kd/pops_04g.prx","ms0:/seplugins/popsloader/modules/610/pops_04g.prx",
			"ms0:/F0/kd/pops_05g.prx","ms0:/seplugins/popsloader/modules/610/pops_05g.prx",
			"ms0:/F0/kd/pops_06g.prx","ms0:/seplugins/popsloader/modules/610/pops_06g.prx",
			"ms0:/F0/kd/pops_07g.prx","ms0:/seplugins/popsloader/modules/610/pops_07g.prx",
			"ms0:/F0/kd/pops_08g.prx","ms0:/seplugins/popsloader/modules/610/pops_08g.prx",
			"ms0:/F0/kd/pops_09g.prx","ms0:/seplugins/popsloader/modules/610/pops_09g.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/610/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/610/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/610/libpspvmc.prx",
		}; Expand("ms0:/610-GO.PBP","ms0:/seplugins/popsloader/modules/610",argc(p610go),p610go);}
	{	char *p620[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/620/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/620/pops.prx",
			"ms0:/F0/kd/pops_01g.prx","ms0:/seplugins/popsloader/modules/620/pops_01g.prx",
			"ms0:/F0/kd/pops_02g.prx","ms0:/seplugins/popsloader/modules/620/pops_02g.prx",
			"ms0:/F0/kd/pops_03g.prx","ms0:/seplugins/popsloader/modules/620/pops_03g.prx",
			"ms0:/F0/kd/pops_04g.prx","ms0:/seplugins/popsloader/modules/620/pops_04g.prx",
			"ms0:/F0/kd/pops_05g.prx","ms0:/seplugins/popsloader/modules/620/pops_05g.prx",
			"ms0:/F0/kd/pops_06g.prx","ms0:/seplugins/popsloader/modules/620/pops_06g.prx",
			"ms0:/F0/kd/pops_07g.prx","ms0:/seplugins/popsloader/modules/620/pops_07g.prx",
			"ms0:/F0/kd/pops_08g.prx","ms0:/seplugins/popsloader/modules/620/pops_08g.prx",
			"ms0:/F0/kd/pops_09g.prx","ms0:/seplugins/popsloader/modules/620/pops_09g.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/620/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/620/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/620/libpspvmc.prx",
		}; Expand("ms0:/620.PBP","ms0:/seplugins/popsloader/modules/620",argc(p620),p620);}
	{	char *p620go[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/620/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/620/pops.prx",
			"ms0:/F0/kd/pops_01g.prx","ms0:/seplugins/popsloader/modules/620/pops_01g.prx",
			"ms0:/F0/kd/pops_02g.prx","ms0:/seplugins/popsloader/modules/620/pops_02g.prx",
			"ms0:/F0/kd/pops_03g.prx","ms0:/seplugins/popsloader/modules/620/pops_03g.prx",
			"ms0:/F0/kd/pops_04g.prx","ms0:/seplugins/popsloader/modules/620/pops_04g.prx",
			"ms0:/F0/kd/pops_05g.prx","ms0:/seplugins/popsloader/modules/620/pops_05g.prx",
			"ms0:/F0/kd/pops_06g.prx","ms0:/seplugins/popsloader/modules/620/pops_06g.prx",
			"ms0:/F0/kd/pops_07g.prx","ms0:/seplugins/popsloader/modules/620/pops_07g.prx",
			"ms0:/F0/kd/pops_08g.prx","ms0:/seplugins/popsloader/modules/620/pops_08g.prx",
			"ms0:/F0/kd/pops_09g.prx","ms0:/seplugins/popsloader/modules/620/pops_09g.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/620/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/620/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/620/libpspvmc.prx",
		}; Expand("ms0:/620-GO.PBP","ms0:/seplugins/popsloader/modules/620",argc(p620go),p620go);}
	{	char *p635[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/635/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/635/pops.prx",
			"ms0:/F0/kd/pops_01g.prx","ms0:/seplugins/popsloader/modules/635/pops_01g.prx",
			"ms0:/F0/kd/pops_02g.prx","ms0:/seplugins/popsloader/modules/635/pops_02g.prx",
			"ms0:/F0/kd/pops_03g.prx","ms0:/seplugins/popsloader/modules/635/pops_03g.prx",
			"ms0:/F0/kd/pops_04g.prx","ms0:/seplugins/popsloader/modules/635/pops_04g.prx",
			"ms0:/F0/kd/pops_05g.prx","ms0:/seplugins/popsloader/modules/635/pops_05g.prx",
			"ms0:/F0/kd/pops_06g.prx","ms0:/seplugins/popsloader/modules/635/pops_06g.prx",
			"ms0:/F0/kd/pops_07g.prx","ms0:/seplugins/popsloader/modules/635/pops_07g.prx",
			"ms0:/F0/kd/pops_08g.prx","ms0:/seplugins/popsloader/modules/635/pops_08g.prx",
			"ms0:/F0/kd/pops_09g.prx","ms0:/seplugins/popsloader/modules/635/pops_09g.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/635/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/635/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/635/libpspvmc.prx",
		}; Expand("ms0:/635.PBP","ms0:/seplugins/popsloader/modules/635",argc(p635),p635);}
	{	char *p635go[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/635/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/635/pops.prx",
			"ms0:/F0/kd/pops_01g.prx","ms0:/seplugins/popsloader/modules/635/pops_01g.prx",
			"ms0:/F0/kd/pops_02g.prx","ms0:/seplugins/popsloader/modules/635/pops_02g.prx",
			"ms0:/F0/kd/pops_03g.prx","ms0:/seplugins/popsloader/modules/635/pops_03g.prx",
			"ms0:/F0/kd/pops_04g.prx","ms0:/seplugins/popsloader/modules/635/pops_04g.prx",
			"ms0:/F0/kd/pops_05g.prx","ms0:/seplugins/popsloader/modules/635/pops_05g.prx",
			"ms0:/F0/kd/pops_06g.prx","ms0:/seplugins/popsloader/modules/635/pops_06g.prx",
			"ms0:/F0/kd/pops_07g.prx","ms0:/seplugins/popsloader/modules/635/pops_07g.prx",
			"ms0:/F0/kd/pops_08g.prx","ms0:/seplugins/popsloader/modules/635/pops_08g.prx",
			"ms0:/F0/kd/pops_09g.prx","ms0:/seplugins/popsloader/modules/635/pops_09g.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/635/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/635/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/635/libpspvmc.prx",
		}; Expand("ms0:/635-GO.PBP","ms0:/seplugins/popsloader/modules/635",argc(p635go),p635go);}
	{	char *p639[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/639/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/639/pops.prx",
			"ms0:/F0/kd/pops_01g.prx","ms0:/seplugins/popsloader/modules/639/pops_01g.prx",
			"ms0:/F0/kd/pops_02g.prx","ms0:/seplugins/popsloader/modules/639/pops_02g.prx",
			"ms0:/F0/kd/pops_03g.prx","ms0:/seplugins/popsloader/modules/639/pops_03g.prx",
			"ms0:/F0/kd/pops_04g.prx","ms0:/seplugins/popsloader/modules/639/pops_04g.prx",
			"ms0:/F0/kd/pops_05g.prx","ms0:/seplugins/popsloader/modules/639/pops_05g.prx",
			"ms0:/F0/kd/pops_06g.prx","ms0:/seplugins/popsloader/modules/639/pops_06g.prx",
			"ms0:/F0/kd/pops_07g.prx","ms0:/seplugins/popsloader/modules/639/pops_07g.prx",
			"ms0:/F0/kd/pops_08g.prx","ms0:/seplugins/popsloader/modules/639/pops_08g.prx",
			"ms0:/F0/kd/pops_09g.prx","ms0:/seplugins/popsloader/modules/639/pops_09g.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/639/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/639/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/639/libpspvmc.prx",
		}; Expand("ms0:/639.PBP","ms0:/seplugins/popsloader/modules/639",argc(p639),p639);}
	{	char *p639go[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/639/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/639/pops.prx",
			"ms0:/F0/kd/pops_01g.prx","ms0:/seplugins/popsloader/modules/639/pops_01g.prx",
			"ms0:/F0/kd/pops_02g.prx","ms0:/seplugins/popsloader/modules/639/pops_02g.prx",
			"ms0:/F0/kd/pops_03g.prx","ms0:/seplugins/popsloader/modules/639/pops_03g.prx",
			"ms0:/F0/kd/pops_04g.prx","ms0:/seplugins/popsloader/modules/639/pops_04g.prx",
			"ms0:/F0/kd/pops_05g.prx","ms0:/seplugins/popsloader/modules/639/pops_05g.prx",
			"ms0:/F0/kd/pops_06g.prx","ms0:/seplugins/popsloader/modules/639/pops_06g.prx",
			"ms0:/F0/kd/pops_07g.prx","ms0:/seplugins/popsloader/modules/639/pops_07g.prx",
			"ms0:/F0/kd/pops_08g.prx","ms0:/seplugins/popsloader/modules/639/pops_08g.prx",
			"ms0:/F0/kd/pops_09g.prx","ms0:/seplugins/popsloader/modules/639/pops_09g.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/639/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/639/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/639/libpspvmc.prx",
		}; Expand("ms0:/639-GO.PBP","ms0:/seplugins/popsloader/modules/639",argc(p639go),p639go);}
	{	char *p660[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/660/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/660/pops.prx",
			"ms0:/F0/kd/pops_01g.prx","ms0:/seplugins/popsloader/modules/660/pops_01g.prx",
			"ms0:/F0/kd/pops_02g.prx","ms0:/seplugins/popsloader/modules/660/pops_02g.prx",
			"ms0:/F0/kd/pops_03g.prx","ms0:/seplugins/popsloader/modules/660/pops_03g.prx",
			"ms0:/F0/kd/pops_04g.prx","ms0:/seplugins/popsloader/modules/660/pops_04g.prx",
			"ms0:/F0/kd/pops_05g.prx","ms0:/seplugins/popsloader/modules/660/pops_05g.prx",
			"ms0:/F0/kd/pops_06g.prx","ms0:/seplugins/popsloader/modules/660/pops_06g.prx",
			"ms0:/F0/kd/pops_07g.prx","ms0:/seplugins/popsloader/modules/660/pops_07g.prx",
			"ms0:/F0/kd/pops_08g.prx","ms0:/seplugins/popsloader/modules/660/pops_08g.prx",
			"ms0:/F0/kd/pops_09g.prx","ms0:/seplugins/popsloader/modules/660/pops_09g.prx",
			"ms0:/F0/kd/pops_10g.prx","ms0:/seplugins/popsloader/modules/660/pops_10g.prx",
			"ms0:/F0/kd/pops_11g.prx","ms0:/seplugins/popsloader/modules/660/pops_11g.prx",
			"ms0:/F0/kd/pops_12g.prx","ms0:/seplugins/popsloader/modules/660/pops_12g.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/660/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/660/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/660/libpspvmc.prx",
		}; Expand("ms0:/660.PBP","ms0:/seplugins/popsloader/modules/660",argc(p660),p660);}
	{	char *p660go[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/660/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/660/pops.prx",
			"ms0:/F0/kd/pops_01g.prx","ms0:/seplugins/popsloader/modules/660/pops_01g.prx",
			"ms0:/F0/kd/pops_02g.prx","ms0:/seplugins/popsloader/modules/660/pops_02g.prx",
			"ms0:/F0/kd/pops_03g.prx","ms0:/seplugins/popsloader/modules/660/pops_03g.prx",
			"ms0:/F0/kd/pops_04g.prx","ms0:/seplugins/popsloader/modules/660/pops_04g.prx",
			"ms0:/F0/kd/pops_05g.prx","ms0:/seplugins/popsloader/modules/660/pops_05g.prx",
			"ms0:/F0/kd/pops_06g.prx","ms0:/seplugins/popsloader/modules/660/pops_06g.prx",
			"ms0:/F0/kd/pops_07g.prx","ms0:/seplugins/popsloader/modules/660/pops_07g.prx",
			"ms0:/F0/kd/pops_08g.prx","ms0:/seplugins/popsloader/modules/660/pops_08g.prx",
			"ms0:/F0/kd/pops_09g.prx","ms0:/seplugins/popsloader/modules/660/pops_09g.prx",
			"ms0:/F0/kd/pops_10g.prx","ms0:/seplugins/popsloader/modules/660/pops_10g.prx",
			"ms0:/F0/kd/pops_11g.prx","ms0:/seplugins/popsloader/modules/660/pops_11g.prx",
			"ms0:/F0/kd/pops_12g.prx","ms0:/seplugins/popsloader/modules/660/pops_12g.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/660/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/660/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/660/libpspvmc.prx",
		}; Expand("ms0:/660-GO.PBP","ms0:/seplugins/popsloader/modules/660",argc(p660go),p660go);}
	{	char *p661[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/661/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/661/pops.prx",
			"ms0:/F0/kd/pops_01g.prx","ms0:/seplugins/popsloader/modules/661/pops_01g.prx",
			"ms0:/F0/kd/pops_02g.prx","ms0:/seplugins/popsloader/modules/661/pops_02g.prx",
			"ms0:/F0/kd/pops_03g.prx","ms0:/seplugins/popsloader/modules/661/pops_03g.prx",
			"ms0:/F0/kd/pops_04g.prx","ms0:/seplugins/popsloader/modules/661/pops_04g.prx",
			"ms0:/F0/kd/pops_05g.prx","ms0:/seplugins/popsloader/modules/661/pops_05g.prx",
			"ms0:/F0/kd/pops_06g.prx","ms0:/seplugins/popsloader/modules/661/pops_06g.prx",
			"ms0:/F0/kd/pops_07g.prx","ms0:/seplugins/popsloader/modules/661/pops_07g.prx",
			"ms0:/F0/kd/pops_08g.prx","ms0:/seplugins/popsloader/modules/661/pops_08g.prx",
			"ms0:/F0/kd/pops_09g.prx","ms0:/seplugins/popsloader/modules/661/pops_09g.prx",
			"ms0:/F0/kd/pops_10g.prx","ms0:/seplugins/popsloader/modules/661/pops_10g.prx",
			"ms0:/F0/kd/pops_11g.prx","ms0:/seplugins/popsloader/modules/661/pops_11g.prx",
			"ms0:/F0/kd/pops_12g.prx","ms0:/seplugins/popsloader/modules/661/pops_12g.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/661/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/661/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/661/libpspvmc.prx",
		}; Expand("ms0:/661.PBP","ms0:/seplugins/popsloader/modules/661",argc(p661),p661);}
	{	char *p661go[]={
			"ms0:/F0/kd/popsman.prx","ms0:/seplugins/popsloader/modules/661/popsman.prx",
			"ms0:/F0/kd/pops.prx","ms0:/seplugins/popsloader/modules/661/pops.prx",
			"ms0:/F0/kd/pops_01g.prx","ms0:/seplugins/popsloader/modules/661/pops_01g.prx",
			"ms0:/F0/kd/pops_02g.prx","ms0:/seplugins/popsloader/modules/661/pops_02g.prx",
			"ms0:/F0/kd/pops_03g.prx","ms0:/seplugins/popsloader/modules/661/pops_03g.prx",
			"ms0:/F0/kd/pops_04g.prx","ms0:/seplugins/popsloader/modules/661/pops_04g.prx",
			"ms0:/F0/kd/pops_05g.prx","ms0:/seplugins/popsloader/modules/661/pops_05g.prx",
			"ms0:/F0/kd/pops_06g.prx","ms0:/seplugins/popsloader/modules/661/pops_06g.prx",
			"ms0:/F0/kd/pops_07g.prx","ms0:/seplugins/popsloader/modules/661/pops_07g.prx",
			"ms0:/F0/kd/pops_08g.prx","ms0:/seplugins/popsloader/modules/661/pops_08g.prx",
			"ms0:/F0/kd/pops_09g.prx","ms0:/seplugins/popsloader/modules/661/pops_09g.prx",
			"ms0:/F0/kd/pops_10g.prx","ms0:/seplugins/popsloader/modules/661/pops_10g.prx",
			"ms0:/F0/kd/pops_11g.prx","ms0:/seplugins/popsloader/modules/661/pops_11g.prx",
			"ms0:/F0/kd/pops_12g.prx","ms0:/seplugins/popsloader/modules/661/pops_12g.prx",
			"ms0:/F0/vsh/module/common_util.prx","ms0:/seplugins/popsloader/modules/661/common_util.prx",
			"ms0:/F0/vsh/module/pafmini.prx","ms0:/seplugins/popsloader/modules/661/pafmini.prx",
			"ms0:/F0/vsh/module/libpspvmc.prx","ms0:/seplugins/popsloader/modules/661/libpspvmc.prx",
		}; Expand("ms0:/661-GO.PBP","ms0:/seplugins/popsloader/modules/661",argc(p661go),p661go);}

	//Add what you want.

	scePowerTick(0);
	//ErrorExit(10000, "Done.\n");
	printf("Auto exit in 3 seconds.\n");
	sceKernelDelayThread(3000000);
	sceKernelExitGame();

    return 0;
}

