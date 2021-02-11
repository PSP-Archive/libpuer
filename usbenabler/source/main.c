#include "../../libpuer/libpuer.h"

PSP_MODULE_INFO("USBEnabler", PSP_MODULE_USER, 1, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);

//#define printf pspDebugScreenPrintf

u32 usbModuleStatus=0;
u32 usbStatus=0;

//based on 3.40OE source code.
int loadstartModule(char *path){
	SceUID mod = sceKernelLoadModule(path, 0, NULL);
	//printf("sceKernelLoadModule(%s)=%08x\n",path,mod);
	if(mod >= 0)
		mod = sceKernelStartModule(mod, strlen(path)+1, path, NULL, NULL);
	return mod;
}

void disableUsb(){
	if(usbStatus){
		//printf("sceUsbDeactivate\n");
		sceUsbDeactivate(0x1c8);
		//printf("sceUsbStop\n");
		sceUsbStop(PSP_USBSTOR_DRIVERNAME, 0, 0);
		sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);
		//printf("pspUsbDeviceFinishDevice\n");
		pspUsbDeviceFinishDevice();
		//printf("finishing...\n");
		usbStatus = 0;
	}
}

void enableUsb(int device, int writeProtectFlash){ //0==MS, 1-4=Flash0-3, 5=UMD
	if(usbStatus){
		disableUsb();
		sceKernelDelayThread(300000);
	}

	if(!usbModuleStatus){
		//if(PSP_INIT_KEYCONFIG_POPS==sceKernelInitKeyConfig()) //==sceKernelApplicationType())
		//	loadstartModule("flash0:/kd/usb.prx");
		if(CFW_PRO)
			loadstartModule("flash0:/kd/_usbdevice.prx");
		else if(CFW_ME)
			loadstartModule("flash0:/kd/usbdev.prx");
		else
			loadstartModule("flash0:/kd/usbdevice.prx");
		loadstartModule("flash0:/kd/semawm.prx");
		loadstartModule("flash0:/kd/usbstor.prx");
		loadstartModule("flash0:/kd/usbstormgr.prx");
		loadstartModule("flash0:/kd/usbstorms.prx");
		loadstartModule("flash0:/kd/usbstoreflash.prx");
		loadstartModule("flash0:/kd/usbstorboot.prx");
		usbModuleStatus = 1;
	}

	writeProtectFlash=!!writeProtectFlash; //set to 1 if not 0
	if(device==5)writeProtectFlash=1;
	if(device)pspUsbDeviceSetDevice(device-1, writeProtectFlash, 0); //ReadWrite unless UMD

	sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
	sceUsbStart(PSP_USBSTOR_DRIVERNAME, 0, 0);
	sceUsbstorBootSetCapacity(0x800000);
	sceUsbActivate(0x1c8);
	usbStatus = 1;
}

#if 0
int main(){
	pspDebugScreenInit();
	SetupExitCallback();

	ResolveDyn();

	enableUsb(0);

	printf("Use home to stop.\n");
	for(;running;)sceKernelDelayThread(1000);

	disableUsb();
	sceKernelExitGame();
    return 0;
}
#endif

int main_returns_immediately=1;
int main(){
	ResolveDyn();
	strcpy(myfile,"usbenabler.ini");
	enableUsb(ini_getl("usbenabler", "device", 0, mypath),ini_getl("usbenabler", "writeProtectFlash", 0, mypath));
	return 0;
}

int finale(){
	disableUsb();
	return 0;
}
