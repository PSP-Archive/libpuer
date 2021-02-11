#include "../../libpuer/libpuer.h"

#ifndef KERNEL
PSP_MODULE_INFO("CtrlCheck", PSP_MODULE_USER, 1, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);
#else
PSP_MODULE_INFO("CtrlCheckK", PSP_MODULE_KERNEL, 1, 1);
PSP_MAIN_THREAD_ATTR(0);
#endif

#define MX 70
#define MY 16
//#define COPY(y,x,str) memcpy(p+(y*MX)+x-(strlen(str)/2),str,strlen(str))

SceCtrlData pad;
char *spaces="                   ";
void COPY(int y,int x,char *str,int key){
	pspDebugScreenSetXY(x-(strlen(str)/2),y*2);
	if(!key || (pad.Buttons & key))pspDebugScreenPrintf(str);
	else pspDebugScreenPrintData(spaces,strlen(str));
}

#ifdef KERNEL
void COPY_MS(int y,int x,char *str){
#if 1
	pspDebugScreenSetXY(x-(strlen(str)/2),y*2);
	if(
		(*((vu32*)0xbd200034)!=0&&*((vu32*)0xbd200034)!=2)&&
		*((vu32*)0xbd200038)!=12328
	)pspDebugScreenPrintf(str);
	else pspDebugScreenPrintData(spaces,strlen(str));
#else
	pspDebugScreenSetXY(0,y*2);
	pspDebugScreenPrintf("%12d %12d %12d %12d",*((vu32*)0xbd200030),*((vu32*)0xbd200034),*((vu32*)0xbd200038),*((vu32*)0xbd20003c));
#endif
}
#endif

int main(){
	pspDebugScreenInit();
	SetupExitCallback();

	pspDebugScreenPrintf(
#ifndef KERNEL
		"*** ctrlcheck ***\n"
#else
		"*** ctrlcheck (K) ***\n"
#endif
		"to stop, use home.\n"
	);
	sceKernelDelayThread(1000000);
	pspDebugScreenInit();

	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

	char X[6],Y[6];
	//char p[MY*MX];
	//int i;
	for(;running;sceDisplayWaitVblankStart()){
		sceCtrlReadBufferPositive(&pad, 1);
		//memset(p,' ',sizeof(p));
		//for(i=1;i<MY;i++)p[i*MX-1]='\n';p[MY*MX-1]=0;
		sprintf(X,"X %03d",pad.Lx);
		sprintf(Y,"Y %03d",pad.Ly);

		COPY(1,10,"LTRIGGER",PSP_CTRL_LTRIGGER);
		COPY(1,30,"WLAN",PSP_CTRL_WLAN_UP);
		COPY(1,40,"Disc",PSP_CTRL_DISC);
		COPY(1,60,"RTRIGGER",PSP_CTRL_RTRIGGER);

		COPY(3,15,"UP",PSP_CTRL_UP);
		COPY(5,10,"LEFT",PSP_CTRL_LEFT);
		COPY(5,20,"RIGHT",PSP_CTRL_RIGHT);
		COPY(7,15,"DOWN",PSP_CTRL_DOWN);

		COPY(3,55,"TRIANGLE",PSP_CTRL_TRIANGLE);
		COPY(5,50,"SQUARE",PSP_CTRL_SQUARE);
		COPY(5,60,"CIRCLE",PSP_CTRL_CIRCLE);
		COPY(7,55,"CROSS",PSP_CTRL_CROSS);

		COPY(7,35,"SLIDE",0x20000000);

#ifdef KERNEL
		COPY_MS(8,10,"[Access]");
#endif
		COPY(9,10,"MS",PSP_CTRL_MS);
		COPY(9,20,X,0);
		COPY(11,20,Y,0);
		COPY(11,60,"Hold",PSP_CTRL_HOLD);

		COPY(13,20,"Home",PSP_CTRL_HOME);
		COPY(13,30,"VolUp",PSP_CTRL_VOLUP);
		COPY(15,30,"VolDown",PSP_CTRL_VOLDOWN);
		COPY(13,50,"Screen",PSP_CTRL_SCREEN);
		COPY(15,50,"Note",PSP_CTRL_NOTE);
		COPY(13,60,"Select",PSP_CTRL_SELECT);
		COPY(15,60,"Start",PSP_CTRL_START);
	}

#ifndef KERNEL
	sceKernelExitGame();
#else
	sceKernelExitVSHVSH(0);
#endif
	return 0;
}
