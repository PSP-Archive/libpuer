#include "../libpuer.h"

#if 0
//This function won't work for PBP... neur0n told me.
//Extracting DATA.PSP isn't helping either...
int executeBasic(const char *path){
	struct SceKernelLoadExecParam param;
	memset(&param,0,sizeof(param));
	param.argp=(char*)path;
	param.args=strlen(path)+1;
	param.key=NULL;//"game";
	param.size=sizeof(param)+param.args;
	return sceKernelLoadExec(path,&param);
}
#endif
