#ifdef SCTRL
#include "../libpuer.h"

//typedef void (*TsctrlSESetBootConfIndex)(int);
//TsctrlSESetBootConfIndex PsctrlSESetBootConfFileIndex;

void SetUmdFile(char*);
void SetBootFile(int);

/*
0 -> normal/OE isofs
1 -> M33
2 -> NP9660
3 -> Inferno/ME
*/

#if 0
int executeISO(const char *path, int mode){
	struct SceKernelLoadExecVSHParam param;
	memset(&param,0,sizeof(param));
	param.argp=(char*)DISC_EBOOT;
	param.args=strlen(DISC_EBOOT)+1;
	param.key="game";
	param.size=sizeof(param);

	SEConfig config;
	sctrlSEGetConfig(&config);

	//sctrlSEUmountUmd();
	//sctrlSESetDiscOut(1);
	if(PsctrlSESetUmdFileEx)
		PsctrlSESetUmdFileEx(path,NULL);
	else
		//sctrlSESetUmdFile(path);
		SetUmdFile(path);
	if(!mode){
		if(config.useisofsonumdinserted)
			sctrlSEMountUmdFromFile(path, 1, 1);
		else
			sctrlSEMountUmdFromFile(path, 0, 0);
	}//else{
	//	sctrlSEMountUmdFromFile(path, 1, 1);
	//}
	sctrlSESetBootConfFileIndex(mode);
	return sctrlKernelLoadExecVSHDisc(DISC_EBOOT, &param);
}
#endif

int executeISO(const char *path, int mode){
	int base;
	//const char *loadexec_file=DISC_EBOOT;

	struct SceKernelLoadExecVSHParam param;
	memset(&param,0,sizeof(param));
	param.argp=(char*)DISC_EBOOT;
	param.args=strlen(DISC_EBOOT)+1;
	//param.key="game";
	//base=0x120;
	param.key="umdemu";
	base=0x123;
	if(kuKernelGetModel() == 4){
		param.key="umdemu";
		base=0x123;
		if(!strncasecmp(path,"ef",2))base=0x125;
		//loadexec_file=path;
	}
	param.size=sizeof(param);

	SEConfig config;
	sctrlSEGetConfig(&config);

	if(PsctrlSESetUmdFileEx)
		PsctrlSESetUmdFileEx((char*)path,NULL);
	else
		//sctrlSESetUmdFile((char*)path);
		SetUmdFile((char*)path);
#if 0
	if(!mode){
		param.key="game";
		base=0x120;
		if(config.useisofsonumdinserted)
			sctrlSEMountUmdFromFile((char*)path, 1, 1);
		else
			sctrlSEMountUmdFromFile((char*)path, 0, 0);
	}
#endif
	//PsctrlSESetBootConfFileIndex=(TsctrlSESetBootConfFileIndex)sctrlHENFindFunction("SystemControl","SystemCtrlForKernel",0x5CB025F0); //asshole
	//PsctrlSESetBootConfFileIndex(mode);
	SetBootFile(mode);
	//sctrlSESetDiscType(0x10);

	return sctrlKernelLoadExecVSHWithApitype(base,DISC_EBOOT,&param);
}

int executeAny(const char* path){
	int type=2;
	int base = 0x13f;
	if(!strncasecmp(path,"ef",2))base=0x150;

	struct SceKernelLoadExecVSHParam param;
	memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.argp = (char*)path;
	param.args = strlen(path)+1;
	param.key = "game";
	//param.vshmain_args_size = 0;
	//param.vshmain_args = NULL;

	{
		char head[40];
		SceUID fd=sceIoOpen(path,PSP_O_RDONLY,0777);
		sceIoRead(fd,head,40);
		if(memcmp(head,"\0PBP",4)){//||read32(head+4)!=0x00010000){
			//sceIoClose(fd);
			goto check_end;
		}

		int param_offset=read32(head+8);
		int param_size=read32(head+12)-param_offset;
		char *p=malloc(align4(param_size));
		sceIoLseek(fd,param_offset,PSP_SEEK_SET);
		sceIoRead(fd,p,param_size);
		if(memcmp(p,"\0PSF",4)||read32(p+4)!=0x00000101){
			free(p);
			goto check_end; //lol
		}
		int label_offset=read32(p+8);
		int data_offset=read32(p+12);
		int nlabel=read32(p+16);
		int i=0;
		for(;i<nlabel;i++){
			if(!strcmp(p+label_offset+read16(p+20+16*i),"DISC_ID")){ //seems to be 16bytes long
				//int datasize=read32(p+20+16*i+8);
				//if(datasize>19)datasize=19;
				if(!memcmp("MSTKUPDATE",p+data_offset+read32(p+20+16*i+12),10)){
					type=1;
					param.key = "updater";
					free(p);
					goto check_end;
				}
			}
		}
		free(p);

		u32 size=sceIoGetFileLength(fd)-read32(head+36);
		if(!size){
			//sceIoClose(fd);
			goto check_end;
		}
		sceIoLseek(fd,read32(head+36),PSP_SEEK_SET);
		sceIoRead(fd,head,16);
		if(!memcmp(head,"PSMULTIIMG",10)||!memcmp(head,"PSISOIMG",8)){
			type = 5;
			param.key = "pops";
			//if(!Pversionspoofer){
			//	executeEBOOT("ms0:/PSP/GAME/FastRecovery/EBOOT.PBP");
			//	sceIoClose(fd);
			//	return -1;
			//}
		}
check_end:
		sceIoClose(fd);
	}

	// sceKernelLoadExecVSHMs5 is unofficial API; need to use sctrlKernelLoadExecVSHWithApitype instead.
	return sctrlKernelLoadExecVSHWithApitype(base+type, path, &param);
#if 0
	int k1 = pspSdkSetK1(0);
	int ret=-1;
	switch(type){
		case 1: ret=sceKernelLoadExecVSHMs1(path, &param);break;
		case 2: ret=sceKernelLoadExecVSHMs2(path, &param);break;
		case 5: ret=sceKernelLoadExecVSHMs4(path, &param);break; //not working
	}
	pspSdkSetK1(k1);
	return ret;
#endif
}

/*
enum PSPLoadModuleApitype{
PSP_LOADMODULE_APITYPE_KERNEL = 0, // ModuleMgrForKernel
PSP_LOADMODULE_APITYPE_USER = 0x10, // ModuleMgrForUser
PSP_LOADMODULE_APITYPE_DNAS = 0x13,
PSP_LOADMODULE_APITYPE_VSH = 0x20,
PSP_LOADMODULE_APITYPE_DISC = 0x120,
PSP_LOADMODULE_APITYPE_DISC_UPDATER = 0x121,
PSP_LOADMODULE_APITYPE_DISC_DEBUG = 0x122,
PSP_LOADMODULE_APITYPE_DISC_IMAGE = 0x123,
PSP_LOADMODULE_APITYPE_USBWLAN = 0x130,
PSP_LOADMODULE_APITYPE_USBWLAN_DEBUG = 0x131,
PSP_LOADMODULE_APITYPE_MS1 = 0x140,
PSP_LOADMODULE_APITYPE_MS2 = 0x141,
PSP_LOADMODULE_APITYPE_MS3 = 0x142,
PSP_LOADMODULE_APITYPE_MS4 = 0x143,
PSP_LOADMODULE_APITYPE_MS5 = 0x144,
PSP_LOADMODULE_APITYPE_VSH_EXIT_VSH_KERNEL = 0x200,
PSP_LOADMODULE_APITYPE_VSH_EXIT_GAME = 0x210,
PSP_LOADMODULE_APITYPE_VSH_EXIT_VSH_VSH = 0x220,
PSP_LOADMODULE_APITYPE_REBOOT_KERNEL = 0x300,
};
*/

#endif
