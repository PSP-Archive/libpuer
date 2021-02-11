#include "../../libpuer/libpuer.h"

#define IMMEDIATE_SHUTDOWN

PSP_MODULE_INFO("PspReboot", PSP_MODULE_USER, 1, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);

#define printf pspDebugScreenPrintf
void scePowerRequestColdReset(int);
int main(){
//#ifdef IMMEDIATE_SHUTDOWN
	scePowerRequestColdReset(0);
//#endif
	sceKernelExitGame();
	return 0;
}

