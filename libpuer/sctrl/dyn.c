#ifdef SCTRL

#include "../libpuer.h"
//#include <systemctrl.h>

int ResolveDyn(){ //must be called in Main, only once
	// *** by the way, although you can use this to check CFW, don't call this pointer from user mode. ***
	if(!isLinked(sctrlHENFindFunction))return 1;

	//codestation: if true, this is PRO.
	Pversionspoofer=(Tversionspoofer)sctrlHENFindFunction("SystemControl", "VersionSpoofer", 0x5B18622C);

	// TN
	PvctrlVSHRegisterVshMenu=(TvctrlVSHRegisterVshMenu)sctrlHENFindFunction("SystemControl", "VshCtrlLib", 0xFD26DA72);

	//neur0ner: if true, this is ME.
	PsctrlSetStartModuleExtra=(TsctrlSetStartModuleExtra)sctrlHENFindFunction("SystemControl", "SystemCtrlForKernel", 0x221400A6);
	PsctrlSESetUmdFileEx=(TsctrlSESetUmdFileEx)sctrlHENFindFunction("SystemControl", "SystemCtrlForKernel", 0xCF817542);

	PsceKernelGetGameInfo=(TsceKernelGetGameInfo)sctrlHENFindFunction("sceSystemMemoryManager", "SysMemForKernel", 0xCD617A94);

	return 0;
}

#endif
