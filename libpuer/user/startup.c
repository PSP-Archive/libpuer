#include "../libpuer.h"

char mypath[768], *myfile;
int running;
int PSP_CTRL_OK,PSP_CTRL_CANCEL;

TsctrlSetStartModuleExtra PsctrlSetStartModuleExtra;
TsctrlSESetUmdFileEx PsctrlSESetUmdFileEx;
Tversionspoofer Pversionspoofer;
TvctrlVSHRegisterVshMenu PvctrlVSHRegisterVshMenu;
TsceKernelGetGameInfo PsceKernelGetGameInfo;

void libpuer_startup(){
	running=1;
	*mypath=0;myfile=mypath;

	PsctrlSetStartModuleExtra=NULL;
	PsctrlSESetUmdFileEx=NULL;
	Pversionspoofer=NULL;
	PvctrlVSHRegisterVshMenu=NULL;
	PsceKernelGetGameInfo=NULL;

	if(argc){
		int i;
		strcpy(mypath, argv[0]);
		for(i=strlen(mypath);i;i--)
			if(mypath[i-1]=='/'){mypath[i]=0;break;}
		myfile=mypath+i;
	}

	PSP_CTRL_OK=0;
	PSP_CTRL_CANCEL=0;

	if(isLinked(sceUtilityGetSystemParamInt)){
		int val;
		sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_UNKNOWN,&val);
		if(!val){
			PSP_CTRL_OK = PSP_CTRL_CIRCLE;
			PSP_CTRL_CANCEL = PSP_CTRL_CROSS;
		}else{
			PSP_CTRL_OK = PSP_CTRL_CROSS;
			PSP_CTRL_CANCEL = PSP_CTRL_CANCEL;
		}
	}
}
