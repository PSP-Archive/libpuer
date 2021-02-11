#include "../../libpuer/libpuer.h"

PSP_MODULE_INFO("WdayLoader", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

#define printf pspDebugScreenPrintf

char *wdays[7]={"sunday","monday","tuesday","wednesday","thursday","friday","saturday"};
char progress[51]="                                                  ";
int main(){
	pspDebugScreenInit();
	SetupExitCallback();

	pspTime pt;
	sceRtcGetCurrentClockLocalTime(&pt);
	int wday=sceRtcGetDayOfWeek(pt.year,pt.month,pt.day);

	for(sceKernelDelayThread(200000);;sceKernelDelayThread(200000)){
		if(!running)sceKernelExitGame();
		sceRtcGetCurrentClockLocalTime(&pt);
		int t=pt.hour*3600+pt.minutes*60+pt.seconds;
		if(t<2)break; //in some lol cases...
		
		pspDebugScreenSetXY(0,0);
		int n=t*50/86400,i=0;
		for(;i<n;i++)progress[i]='*';
		printf(
			"Loading wd0:/%s.prx %02d%%... Remaining %02d:%02d:%02d\n"
			"[%s]\n"
			,wdays[(wday+1)%7],t*100/86400,(86400-t)/3600,(86400-t)/60%60,(86400-t)%60,progress
		);
	}

	pspDebugScreenSetXY(0,0);
	int n=50,i=0;
	for(;i<n;i++)progress[i]='*';
	printf(
		"Loaded wd0:/%s.prx 100%%!   Remaining 00:00:00\n"
		"[%s]\n"
		,wdays[(wday+1)%7],progress
	);

	for(sceKernelDelayThread(500000);running;sceKernelDelayThread(500000));

	sceKernelExitGame();
    return 0;
}
