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

PSP_MODULE_INFO("500DC8ex", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

#define KERNELVERSION (sceKernelDevkitVersion()>>8)
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
#if 0
static char _10g_table[0x4000];
static int _10gtable_size;

static char _11g_table[0x4000];
static int _11gtable_size;

static char _12g_table[0x4000];
static int _12gtable_size;
#endif
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

	printf("5.00 DC8 Extreme based on PSAR Dumper by PspPet\n");
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

	char *pbpname=NULL,*music_pbpname=NULL,*prome_pbpname=NULL;
	int can_extract=0;
	SceUID mod;
	if (KERNELVERSION < 0x020701){
		printf("Dumping OFW requires CFW 2.71 or higher.\n");
	}else if((mod = LoadStartModule("libpsardumper.prx", PSP_MEMORY_PARTITION_KERNEL))<0){
		printf("Error 0x%08X loading/starting libpsardumper.prx.\n", mod);
	//}else if((mod = LoadStartModule("pspdecrypt.prx", PSP_MEMORY_PARTITION_KERNEL))<0){
	//	printf("Error 0x%08X loading/starting pspdecrypt.prx.\n", mod);
	}else{
		int fd;

		fd=sceIoOpen("ms0:/500.PBP",PSP_O_RDONLY,0777);
		if(fd>=0){
			sceIoClose(fd);
			pbpname="ms0:/500.PBP";
		}

		fd=sceIoOpen("ms0:/550.PBP",PSP_O_RDONLY,0777);
		if(fd>=0){
			sceIoClose(fd);
			music_pbpname="ms0:/550.PBP";
		}
		fd=sceIoOpen("ms0:/551.PBP",PSP_O_RDONLY,0777);
		if(fd>=0){
			sceIoClose(fd);
			music_pbpname="ms0:/551.PBP";
		}

		fd=sceIoOpen("ms0:/631.PBP",PSP_O_RDONLY,0777);
		if(fd>=0){
			sceIoClose(fd);
			prome_pbpname="ms0:/631.PBP";
		}
		fd=sceIoOpen("ms0:/635.PBP",PSP_O_RDONLY,0777);
		if(fd>=0){
			sceIoClose(fd);
			prome_pbpname="ms0:/635.PBP";
		}
	}
	if(pbpname && music_pbpname && prome_pbpname)can_extract=1;

	printf("Press triangle to check prome3 modules.\n");
	if(can_extract)printf("Press cross to start dumping 5.00DC8ex modules.\n");
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
			check("prome3.md5sum");
			printf("Auto exit in 3 seconds.\n");
			sceKernelDelayThread(3000000);
			sceKernelExitGame();
		}

		sceKernelDelayThread(1000);
	}
	sceIoMkdir("ms0:/TM", 0777);
	sceIoMkdir("ms0:/TM/DC8", 0777);
	sceIoMkdir("ms0:/TM/DC8/6xxModules", 0777);
	sceIoMkdir("ms0:/TM/DC8/data", 0777);
	sceIoMkdir("ms0:/TM/DC8/data/cert", 0777);
	sceIoMkdir("ms0:/TM/DC8/dic", 0777);
	sceIoMkdir("ms0:/TM/DC8/font", 0777);
	sceIoMkdir("ms0:/TM/DC8/gps", 0777);
	sceIoMkdir("ms0:/TM/DC8/kd", 0777);
	sceIoMkdir("ms0:/TM/DC8/kd/resource", 0777);
	sceIoMkdir("ms0:/TM/DC8/net", 0777);
	sceIoMkdir("ms0:/TM/DC8/net/http", 0777);
	sceIoMkdir("ms0:/TM/DC8/registry", 0777);
	sceIoMkdir("ms0:/TM/DC8/vsh", 0777);
	sceIoMkdir("ms0:/TM/DC8/vsh/etc", 0777);
	sceIoMkdir("ms0:/TM/DC8/vsh/module", 0777);
	sceIoMkdir("ms0:/TM/DC8/vsh/resource", 0777);
	sceIoMkdir("ms0:/TM/DC8/vsh/theme", 0777);

	sceKernelVolatileMemLock(0, (void *)&g_dataOut2, &s);

/*
	ExtractReboot(mode, "ms0:/TM/DC8/kd/loadexec.prx", "ms0:/TM/DC8/reboot.bin", "reboot.bin");
	ExtractReboot(mode, "ms0:/TM/DC8/kd/loadexec_01g.prx", "ms0:/TM/DC8/reboot.bin", "reboot.bin");
	ExtractReboot(mode, "ms0:/TM/DC8/kd/loadexec_02g.prx", "ms0:/TM/DC8/reboot_02g.bin", "reboot_02g.bin");
	ExtractReboot(mode, "ms0:/TM/DC8/kd/loadexec_03g.prx", "ms0:/TM/DC8/reboot_03g.bin", "reboot_03g.bin");
	ExtractReboot(mode, "ms0:/TM/DC8/kd/loadexec_04g.prx", "ms0:/TM/DC8/reboot_04g.bin", "reboot_04g.bin");
	ExtractReboot(mode, "ms0:/TM/DC8/kd/loadexec_05g.prx", "ms0:/TM/DC8/reboot_05g.bin", "reboot_05g.bin");
	ExtractReboot(mode, "ms0:/TM/DC8/kd/loadexec_06g.prx", "ms0:/TM/DC8/reboot_06g.bin", "reboot_06g.bin");
	ExtractReboot(mode, "ms0:/TM/DC8/kd/loadexec_07g.prx", "ms0:/TM/DC8/reboot_07g.bin", "reboot_07g.bin");
	ExtractReboot(mode, "ms0:/TM/DC8/kd/loadexec_08g.prx", "ms0:/TM/DC8/reboot_08g.bin", "reboot_08g.bin");
	ExtractReboot(mode, "ms0:/TM/DC8/kd/loadexec_09g.prx", "ms0:/TM/DC8/reboot_09g.bin", "reboot_09g.bin");
*/

	{	/*char *p[]={ //extract all 5.00 modules
		};*/ Expand(pbpname,MODE_ENCRYPT,0,NULL);}

	{	char *pMUSIC[]={
			"ms0:/TM/DC8/vsh/module/content_browser.prx","ms0:/TM/DC8/vsh/module/content_browser.prx",
			"ms0:/TM/DC8/vsh/module/mp4msv.prx","ms0:/TM/DC8/vsh/module/mp4msv.prx",
			"ms0:/TM/DC8/vsh/module/msvideo_main_plugin.prx","ms0:/TM/DC8/vsh/module/msvideo_main_plugin.prx",
			"ms0:/TM/DC8/vsh/module/msvideo_plugin.prx","ms0:/TM/DC8/vsh/module/msvideo_plugin.prx",
			"ms0:/TM/DC8/vsh/module/music_browser.prx","ms0:/TM/DC8/vsh/module/music_browser.prx",
			"ms0:/TM/DC8/vsh/module/music_main_plugin.prx","ms0:/TM/DC8/vsh/module/music_main_plugin.prx",
			"ms0:/TM/DC8/vsh/module/music_parser.prx","ms0:/TM/DC8/vsh/module/music_parser.prx",
			"ms0:/TM/DC8/vsh/module/music_player.prx","ms0:/TM/DC8/vsh/module/music_player.prx",
			"ms0:/TM/DC8/vsh/module/photo_browser.prx","ms0:/TM/DC8/vsh/module/photo_browser.prx",
			"ms0:/TM/DC8/vsh/module/photo_main_plugin.prx","ms0:/TM/DC8/vsh/module/photo_main_plugin.prx",
			"ms0:/TM/DC8/vsh/module/photo_player.prx","ms0:/TM/DC8/vsh/module/photo_player.prx",
			"ms0:/TM/DC8/vsh/module/video_main_plugin.prx","ms0:/TM/DC8/vsh/module/video_main_plugin.prx",
			"ms0:/TM/DC8/vsh/module/video_plugin.prx","ms0:/TM/DC8/vsh/module/video_plugin.prx",
			"ms0:/TM/DC8/vsh/module/visualizer_plugin.prx","ms0:/TM/DC8/vsh/module/visualizer_plugin.prx",
			"ms0:/TM/DC8/vsh/resource/content_browser_plugin.rco","ms0:/TM/DC8/vsh/resource/content_browser_plugin.rco",
			"ms0:/TM/DC8/vsh/resource/msvideo_main_plugin.rco","ms0:/TM/DC8/vsh/resource/msvideo_main_plugin.rco",
			"ms0:/TM/DC8/vsh/resource/music_browser_plugin.rco","ms0:/TM/DC8/vsh/resource/music_browser_plugin.rco",
			"ms0:/TM/DC8/vsh/resource/music_player_plugin.rco","ms0:/TM/DC8/vsh/resource/music_player_plugin.rco",
			"ms0:/TM/DC8/vsh/resource/photo_browser_plugin.rco","ms0:/TM/DC8/vsh/resource/photo_browser_plugin.rco",
			"ms0:/TM/DC8/vsh/resource/photo_player_plugin.rco","ms0:/TM/DC8/vsh/resource/photo_player_plugin.rco",
			"ms0:/TM/DC8/vsh/resource/video_main_plugin.rco","ms0:/TM/DC8/vsh/resource/video_main_plugin.rco",
			"ms0:/TM/DC8/vsh/resource/video_plugin_videotoolbar.rco","ms0:/TM/DC8/vsh/resource/video_plugin_videotoolbar.rco",
			"ms0:/TM/DC8/vsh/resource/visualizer_plugin.rco","ms0:/TM/DC8/vsh/resource/visualizer_plugin.rco",
		}; Expand(music_pbpname,MODE_DECRYPT,argc(pMUSIC),pMUSIC);}

	{	char *pPROME[]={
			"ms0:/TM/DC8/kd/audiocodec_260.prx","ms0:/TM/DC8/6xxModules/audiocodec_260.prx",
			"ms0:/TM/DC8/kd/avcodec.prx","ms0:/TM/DC8/6xxModules/avcodec.prx",
			"ms0:/TM/DC8/kd/g729.prx","ms0:/TM/DC8/6xxModules/g729.prx",
			"ms0:/TM/DC8/kd/libaac.prx","ms0:/TM/DC8/6xxModules/libaac.prx",
			"ms0:/TM/DC8/kd/libatrac3plus.prx","ms0:/TM/DC8/6xxModules/libatrac3plus.prx",
			"ms0:/TM/DC8/kd/libmp3.prx","ms0:/TM/DC8/6xxModules/libmp3.prx",
			"ms0:/TM/DC8/kd/libmp4.prx","ms0:/TM/DC8/6xxModules/libmp4.prx",
			"ms0:/TM/DC8/kd/me_wrapper.prx","ms0:/TM/DC8/6xxModules/me_wrapper.prx",
			"ms0:/TM/DC8/kd/mp4msv.prx","ms0:/TM/DC8/6xxModules/mp4msv.prx",
			"ms0:/TM/DC8/kd/mpeg.prx","ms0:/TM/DC8/6xxModules/mpeg.prx",
			"ms0:/TM/DC8/kd/mpegbase_260.prx","ms0:/TM/DC8/6xxModules/mpegbase_260.prx",
			"ms0:/TM/DC8/kd/sc_sascore.prx","ms0:/TM/DC8/6xxModules/sc_sascore.prx",
			"ms0:/TM/DC8/kd/vaudio.prx","ms0:/TM/DC8/6xxModules/vaudio.prx",
			"ms0:/TM/DC8/kd/videocodec_260.prx","ms0:/TM/DC8/6xxModules/videocodec_260.prx",
			"ms0:/TM/DC8/kd/resource/me_t2img.img","ms0:/TM/DC8/6xxModules/me_t2img.img",
			"ms0:/TM/DC8/kd/resource/meimg.img","ms0:/TM/DC8/6xxModules/meimg.img",
		}; Expand(prome_pbpname,MODE_DECRYPT,argc(pPROME),pPROME);}

	//tweaking...
	printf("Finalizing...\n");
	sceIoRemove("ms0:/TM/DC8/vsh/resource/bg.bmp");
	sceIoRename("ms0:/TM/DC8/vsh/resource/01-12_03g.bmp","ms0:/TM/DC8/vsh/resource/bg.bmp");
	copy("flash2:/act.dat","ms0:/TM/DC8/act.dat");

	strcpy(myfile,"res/ipl.bin");copy(mypath,"ms0:/TM/DC8/ipl.bin");
	strcpy(myfile,"res/ipl_01g.bin");copy(mypath,"ms0:/TM/DC8/ipl_01g.bin");
	strcpy(myfile,"res/ipl_02g.bin");copy(mypath,"ms0:/TM/DC8/ipl_02g.bin");
	strcpy(myfile,"res/tmctrl500.prx");copy(mypath,"ms0:/TM/DC8/tmctrl500.prx");

	strcpy(myfile,"res/prometheus_key.txt");copy(mypath,"ms0:/TM/DC8/prometheus_key.txt");
	strcpy(myfile,"res/stargate.prx");copy(mypath,"ms0:/TM/DC8/kd/stargate.prx");
	strcpy(myfile,"res/prometheus_lite.prx");copy(mypath,"ms0:/TM/DC8/kd/prometheus_lite.prx");
	strcpy(myfile,"res/march33.prx");copy(mypath,"ms0:/TM/DC8/kd/march33.prx");
	strcpy(myfile,"res/resurrection.prx");copy(mypath,"ms0:/TM/DC8/kd/resurrection.prx");
	strcpy(myfile,"res/systemctrl.prx");copy(mypath,"ms0:/TM/DC8/kd/systemctrl.prx");
	strcpy(myfile,"res/systemctrl_02g.prx");copy(mypath,"ms0:/TM/DC8/kd/systemctrl_02g.prx");
	strcpy(myfile,"res/leda.prx");copy(mypath,"ms0:/TM/DC8/kd/leda.prx");

	strcpy(myfile,"res/dcman.prx");copy(mypath,"ms0:/TM/DC8/kd/dcman.prx");
	strcpy(myfile,"res/emc_sm_updater.prx");copy(mypath,"ms0:/TM/DC8/kd/emc_sm_updater.prx");
	strcpy(myfile,"res/galaxy.prx");copy(mypath,"ms0:/TM/DC8/kd/galaxy.prx");
	strcpy(myfile,"res/idcanager.prx");copy(mypath,"ms0:/TM/DC8/kd/idcanager.prx");
	strcpy(myfile,"res/idsregeneration.prx");copy(mypath,"ms0:/TM/DC8/kd/idsregeneration.prx");
	strcpy(myfile,"res/iop.prx");copy(mypath,"ms0:/TM/DC8/kd/iop.prx");
	strcpy(myfile,"res/ipl_update.prx");copy(mypath,"ms0:/TM/DC8/kd/ipl_update.prx");
	strcpy(myfile,"res/lfatfs_updater.prx");copy(mypath,"ms0:/TM/DC8/kd/lfatfs_updater.prx");
	strcpy(myfile,"res/lflash_fatfmt_updater.prx");copy(mypath,"ms0:/TM/DC8/kd/lflash_fatfmt_updater.prx");
	strcpy(myfile,"res/lflash_fdisk.prx");copy(mypath,"ms0:/TM/DC8/kd/lflash_fdisk.prx");
	strcpy(myfile,"res/libpsardumper.prx");copy(mypath,"ms0:/TM/DC8/kd/libpsardumper.prx");
	strcpy(myfile,"res/popcorn.prx");copy(mypath,"ms0:/TM/DC8/kd/popcorn.prx");
	strcpy(myfile,"res/pspdecrypt.prx");copy(mypath,"ms0:/TM/DC8/kd/pspdecrypt.prx");
	strcpy(myfile,"res/usbdevice.prx");copy(mypath,"ms0:/TM/DC8/kd/usbdevice.prx");
	strcpy(myfile,"res/vshctrl.prx");copy(mypath,"ms0:/TM/DC8/kd/vshctrl.prx");

	strcpy(myfile,"res/pspbtdnf.bin");copy(mypath,"ms0:/TM/DC8/kd/pspbtdnf.bin");
	strcpy(myfile,"res/pspbtdnf_02g.bin");copy(mypath,"ms0:/TM/DC8/kd/pspbtdnf_02g.bin");
	strcpy(myfile,"res/pspbtjnf.bin");copy(mypath,"ms0:/TM/DC8/kd/pspbtjnf.bin");
	strcpy(myfile,"res/pspbtjnf_02g.bin");copy(mypath,"ms0:/TM/DC8/kd/pspbtjnf_02g.bin");
	strcpy(myfile,"res/pspbtknf.bin");copy(mypath,"ms0:/TM/DC8/kd/pspbtknf.bin");
	strcpy(myfile,"res/pspbtknf_02g.bin");copy(mypath,"ms0:/TM/DC8/kd/pspbtknf_02g.bin");
	strcpy(myfile,"res/pspbtlnf.bin");copy(mypath,"ms0:/TM/DC8/kd/pspbtlnf.bin");
	strcpy(myfile,"res/pspbtlnf_02g.bin");copy(mypath,"ms0:/TM/DC8/kd/pspbtlnf_02g.bin");

	strcpy(myfile,"res/intraFont.prx");copy(mypath,"ms0:/TM/DC8/vsh/module/intraFont.prx");
	strcpy(myfile,"res/recovery.prx");copy(mypath,"ms0:/TM/DC8/vsh/module/recovery.prx");
	strcpy(myfile,"res/satelite.prx");copy(mypath,"ms0:/TM/DC8/vsh/module/satelite.prx");
	strcpy(myfile,"res/vlf.prx");copy(mypath,"ms0:/TM/DC8/vsh/module/vlf.prx");
	strcpy(myfile,"res/version.txt");copy(mypath,"ms0:/TM/DC8/vsh/etc/version.txt");

	scePowerTick(0);
	//ErrorExit(10000, "Done.\n");
	printf("Auto exit in 3 seconds.\n");
	sceKernelDelayThread(3000000);
	sceKernelExitGame();

    return 0;
}

