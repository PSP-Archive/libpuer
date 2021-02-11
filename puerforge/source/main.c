#include "../../libpuer/libpuer.h"

//#define IMMEDIATE_SHUTDOWN

PSP_MODULE_INFO("PuerForge", PSP_MODULE_KERNEL, 1, 1);
//PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);
PSP_MAIN_THREAD_ATTR(0);

#define printf pspDebugScreenPrintf

int LoadStartModule(char *module, int partition){
	//SceUID mod = kuKernelLoadModule(module, 0, NULL);
	SceUID mod = sceKernelLoadModule(module, 0, NULL);

	if (mod < 0)
		return mod;

	return sceKernelStartModule(mod, 0, NULL, NULL, NULL);
}

int main(){
#ifdef IMMEDIATE_SHUTDOWN
	scePowerRequestStandby();
#else
	pspDebugScreenInit();
SetupExitCallback();
	//SceUID mod = LoadStartModule("libpsardumper.prx", PSP_MEMORY_PARTITION_KERNEL);
	//if (mod < 0)
	//{
	//	ErrorExit(5000, "Error 0x%08X loading/starting libpsardumper.prx.\n", mod);
	//}

	//mod = LoadStartModule("pspdecrypt.prx", PSP_MEMORY_PARTITION_KERNEL);
	//if (mod < 0)
	//{
	//	ErrorExit(5000, "Error 0x%08X loading/starting pspdecrypt.prx.\n", mod);
	//}

	struct KernelCallArg kcall;memset(&kcall,0,sizeof(struct KernelCallArg));
	typedef int (*TkuKernelCall)(void *func_addr, struct KernelCallArg *args);
	TkuKernelCall PkuKernelCall=NULL;
	typedef int (*TkuKernelGetModel)();
	TkuKernelGetModel PkuKernelGetModel=NULL;
	if(isLinked(sctrlHENFindFunction)){
		PkuKernelCall=(TkuKernelCall)sctrlHENFindFunction("SystemControl","KUBridge",0x9060F69D);
		PkuKernelGetModel=(TkuKernelGetModel)sctrlHENFindFunction("SystemControl","KUBridge",toNid("kuKernelGetModel"));//0x24331850);
	}else{
		printf("sctrlHENFindFunction not linked!\n");
	}
	printf("toNid(kuKernelGetModel) %08X\n",toNid("kuKernelGetModel"));
	printf("PSP_CTRL_OK:            %08X\n",PSP_CTRL_OK);
	printf("PkuKernelCall:          %08X\n",(u32)PkuKernelCall);
	printf("PkuKernelGetModel:      %08X\n\n",(u32)PkuKernelGetModel);

	printf("*sctrlHENFindFunction:  %08X %08X %08X\n",(u32)sctrlHENFindFunction,
	((u32*)sctrlHENFindFunction)[0],((u32*)sctrlHENFindFunction)[1]);
	printf("*kuKernelGetModel:      %08X %08X %08X\n",(u32)kuKernelGetModel,
	((u32*)kuKernelGetModel)[0],((u32*)kuKernelGetModel)[1]);

	// I don't understand how this works fully...
	typedef int (*TsceKernelGetModel)();
	TsceKernelGetModel PsceKernelGetModel;
	u32 NID=getNid(0x6373995D,	0xDA07DC6E,	0x864EBFD7,	0x458A70B5,	0x07C586A1);
	PsceKernelGetModel=(TsceKernelGetModel)sctrlHENFindFunction("sceSystemMemoryManager","SysMemForKernel",NID);
	printf("PsceKernelGetModel:     [%08X] %08X\n\n",NID,(u32)PsceKernelGetModel);

	printf("kuKernelGetModel()      -> %d\n",kuKernelGetModel());
	if(PkuKernelGetModel)
	printf("PkuKernelGetModel()     -> %d\n",PkuKernelGetModel());
	if(PkuKernelCall){
	PkuKernelCall(PsceKernelGetModel,&kcall);
	printf("PkuKernelCall(PsceKernelGetModel,&kcall) -> %d\n",kcall.ret1);
	}

retry:
	printf("\n");
	printf("[Home]:   return to XMB\n");
	printf("Cross:    shutdown PSP\n");
	printf("Triangle: reboot PSP\n");
	printf("Square:   test C++ call\n");
	printf("\n");

	for(;;){
		if(!running)sceKernelExitVSHVSH(0); //sceKernelExitGame() isn't working lol...

		SceCtrlData pad;
		sceCtrlReadBufferPositive(&pad, 1);

		//if(pad.Buttons & PSP_CTRL_CIRCLE){
		//	sceKernelExitGame();
		//	break;
		//}

		if(pad.Buttons & PSP_CTRL_CROSS){
			scePowerRequestStandby();
			break;
		}

#if 1
		if(pad.Buttons & PSP_CTRL_TRIANGLE){
			typedef void(*TscePowerRequestColdReset)(int);
			TscePowerRequestColdReset PscePowerRequestColdReset;
			u32 NID=getNid(0x0442D852,0x80A2AAD4,0x599A2901,0xF50DA013,0x9DAF25A0);
			PscePowerRequestColdReset=(TscePowerRequestColdReset)sctrlHENFindFunction("scePower_Service","scePower_driver",NID);
			printf("PscePowerRequestColdReset: [%08X] %08X\n",NID,(u32)PscePowerRequestColdReset);
			//if(addr>0){ //it fails... it is kernel mode function...
			//int k1=pspSdkSetK1(0);
				PscePowerRequestColdReset(0);
				//PkuKernelCall(PscePowerRequestColdReset,&kcall);
			//pspSdkSetK1(k1);
			//}
			break;
		}

		if(pad.Buttons & PSP_CTRL_START){
		}
#endif

		if(pad.Buttons & PSP_CTRL_SQUARE){
			extern void reversepolandTest();
			reversepolandTest();
			goto retry;
//void ClearCaches(){
	//int k1=pspSdkSetK1(0);
	//sceKernelIcacheInvalidateAll();
	//sceKernelDcacheWritebackInvalidateAll();
	//pspSdkSetK1(k1);
//}
			//break;
		}
		
		sceKernelDelayThread(1000);
	}

#endif

	sceKernelExitVSHVSH(0); //sceKernelExitGame() isn't working lol...
    return 0;
}

