#include "../../libpuer/libpuer.h"

#define IMMEDIATE_SHUTDOWN

PSP_MODULE_INFO("PspShutdown", PSP_MODULE_USER, 1, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);

#define printf pspDebugScreenPrintf

int main(){
//#ifdef IMMEDIATE_SHUTDOWN
	scePowerRequestStandby();
//#endif
	sceKernelExitGame();
	return 0;
}

