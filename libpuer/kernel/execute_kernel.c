#include "../libpuer.h"

int executeEBOOT(const char* path){
	struct SceKernelLoadExecVSHParam param;

	memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.argp = (char*)path;
	param.args = strlen(path)+1;
	param.key = "game";
	//param.vshmain_args_size = 0;
	//param.vshmain_args = NULL;
	int k1 = pspSdkSetK1(0);
	int ret = sceKernelLoadExecVSHMs2(path, &param);
	pspSdkSetK1(k1);
	return ret;
}

/*
int executeEBOOT2(const char* path){
	u32 vshmain_args[0x400/4];
	memset(vshmain_args, 0, sizeof(vshmain_args));
	vshmain_args[0x40] = 1;
	vshmain_args[0x280] = 1;
	vshmain_args[0x284] = 3;
	vshmain_args[0x286] = 5;

	struct SceKernelLoadExecVSHParam param;

	memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.argp = (char*)path;
	param.args = strlen(path)+1;
	param.key = "game";
	param.vshmain_args_size = sizeof(vshmain_args);
	param.vshmain_args = vshmain_args;
	int k1 = pspSdkSetK1(0);
	int ret = sceKernelLoadExecVSHMs2(path, &param);
	pspSdkSetK1(k1);
	return ret;
}
*/

int executeUpdater(const char* path){
	struct SceKernelLoadExecVSHParam param;

	memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.argp = (char*)path;
	param.args = strlen(path)+1;
	param.key = NULL;
	//param.vshmain_args_size = 0;
	//param.vshmain_args = NULL;
	int k1 = pspSdkSetK1(0);
	int ret = sceKernelLoadExecVSHMs1(path, &param);
	pspSdkSetK1(k1);
	return ret;
}

int executeUMD(){
	struct SceKernelLoadExecVSHParam param;
	memset(&param,0,sizeof(param));
	param.argp=(char*)DISC_EBOOT;
	param.args=strlen(DISC_EBOOT)+1;
	param.key="game";
	param.size=sizeof(param);

	if(sceUmdCheckMedium())return -1;
	sceUmdWaitDriveStat(UMD_WAITFORDISC);
	sceUmdActivate(1, "disc0:");
	int k1 = pspSdkSetK1(0);
	int ret = sceKernelLoadExecVSHDisc(DISC_EBOOT, &param);
	pspSdkSetK1(k1);
	return ret;
}
