#include "../../libpuer/libpuer.h"

PSP_MODULE_INFO("autobranch", PSP_MODULE_KERNEL, 1, 1);
mainThreadParam("autobranch",8,256,0);

int main(){
	char path[768];
	for(;;sceKernelDelayThread(1000))if(
			sceKernelFindModuleByName("sceSystemMemoryManager") && 
			sceKernelFindModuleByName("sceIOFileManager") && 
			sceKernelFindModuleByName("sceGE_Manager") && 
			sceKernelFindModuleByName("sceDisplay_Service") && 
			sceKernelFindModuleByName("sceController_Service") && 
			sceKernelFindModuleByName("sceKernelLibrary")
	)break;

	strcpy(myfile,"autobranch.ini");
	debug("ResolveDyn\n");
	ResolveDyn();
	debug("PsctrlSESetUmdFileEx=%08x\n",PsctrlSESetUmdFileEx);

	SceCtrlData pad;
	sceCtrlReadBufferPositive(&pad, 1);
	sceKernelDelayThread(10000);

	*path=0;
	if(pad.Buttons & PSP_CTRL_CIRCLE)
		ini_gets("autobranch", "CIRCLE", "", path, 768, mypath);
	if(pad.Buttons & PSP_CTRL_CROSS)
		ini_gets("autobranch", "CROSS", "", path, 768, mypath);
	if(pad.Buttons & PSP_CTRL_TRIANGLE)
		ini_gets("autobranch", "TRIANGLE", "", path, 768, mypath);
	if(pad.Buttons & PSP_CTRL_SQUARE)
		ini_gets("autobranch", "SQUARE", "", path, 768, mypath);
	if(pad.Buttons & PSP_CTRL_UP)
		ini_gets("autobranch", "UP", "", path, 768, mypath);
	if(pad.Buttons & PSP_CTRL_DOWN)
		ini_gets("autobranch", "DOWN", "", path, 768, mypath);
	if(pad.Buttons & PSP_CTRL_LEFT)
		ini_gets("autobranch", "LEFT", "", path, 768, mypath);
	if(pad.Buttons & PSP_CTRL_RIGHT)
		ini_gets("autobranch", "RIGHT", "", path, 768, mypath);
	if(pad.Buttons & PSP_CTRL_LTRIGGER)
		ini_gets("autobranch", "LTRIGGER", "", path, 768, mypath);
	if(pad.Buttons & PSP_CTRL_RTRIGGER)
		ini_gets("autobranch", "RTRIGGER", "", path, 768, mypath); //shouldn't be used; usually means Recovery Menu
	if(pad.Buttons & PSP_CTRL_START)
		ini_gets("autobranch", "START", "", path, 768, mypath);
	if(pad.Buttons & PSP_CTRL_SELECT)
		ini_gets("autobranch", "SELECT", "", path, 768, mypath);
	if(pad.Buttons & PSP_CTRL_NOTE)
		ini_gets("autobranch", "NOTE", "", path, 768, mypath);
	if(pad.Buttons & PSP_CTRL_HOME)
		ini_gets("autobranch", "HOME", "", path, 768, mypath); //shouldn't be used; usually means OFW mode

	int iso_mode=1;
	if(CFW_ME){
		iso_mode=ini_getl("autobranch", "ISOMode_ME", 2, mypath);
	}else if(CFW_PRO){
		iso_mode=ini_getl("autobranch", "ISOMode_PRO", 3, mypath);
	}else{
		iso_mode=ini_getl("autobranch", "ISOMode_Other", 1, mypath);
	}

	if(*path){
		if(!strcasecmp(getextname(path),".iso")||!strcasecmp(getextname(path),".cso")||!strcasecmp(getextname(path),".jso")||!strcasecmp(getextname(path),".dax")){
			debug("executeISO(%s,%d)\n",path,iso_mode);
			executeISO(path,iso_mode);
		}else{
			executeAny(path);
		}
	}
	return 0;
}

//int prelude(){debug_waitpsplink();}
