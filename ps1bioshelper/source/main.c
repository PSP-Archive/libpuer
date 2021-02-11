#include "../../libpuer/libpuer.h"

PSP_MODULE_INFO("PS1BiosHelper", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

#define printf pspDebugScreenPrintf

int main(){
	char memcard[768];
	pspDebugScreenInit();pspDebugScreenSetXY(0,0);
	SetupExitCallback();

	printf(
		"PS1 Bios Dumper Helper\n"
	);

	strcpy(myfile,"SCPHXXXXX.bin");
	FILE *f=fopen(mypath,"r+b");
	if(!f){
		f=fopen(mypath,"wb");
		memset(libpuer_buf,0,sizeof(BUFLEN));
		int i=512*1024;
		for(;i;i-=min(BUFLEN,i))fwrite(libpuer_buf,1,min(BUFLEN,i),f);
		fclose(f);
		f=fopen(mypath,"r+b");
	}
	if(!f){
		printf("cannot open SCPHXXXXX.bin\n");
		for(;;sceKernelDelayThread(250000))if(!running)sceKernelExitGame();
	}

	strcpy(myfile,"mcrpath.txt");
	FILE *mcr=fopen(mypath,"rb");
	if(!mcr){
		fclose(f);
		printf("cannot open mcrpath.txt\n");
		for(;;sceKernelDelayThread(250000))if(!running)sceKernelExitGame();
	}
	myfgets(memcard,768,mcr);
	fclose(mcr);
	printf("Memory Card: %s\n",memcard);
	mcr=fopen(memcard,"rb");
	if(!mcr){
		fclose(f);
		printf("cannot open memory card\n");
		for(;;sceKernelDelayThread(250000))if(!running)sceKernelExitGame();
	}

	int length=filelength(fileno(mcr)),vmpofs=0,ofs=0,size=0;
	if(length==131072)printf("Detected MCR.\n");
	else if(length==131200)printf("Detected VMP.\n"),vmpofs=0x80;
	else{
		fclose(f);fclose(mcr);
		printf("file size is invalid\n");
		for(;;sceKernelDelayThread(250000))if(!running)sceKernelExitGame();
	}
	fseek(mcr,0x96+vmpofs,SEEK_SET);
	fread(libpuer_buf,1,7,mcr);
	if(memcmp(libpuer_buf,"BIOSPT",6)){
		fclose(f);fclose(mcr);
		printf("signature is invalid\n");
		for(;;sceKernelDelayThread(250000))if(!running)sceKernelExitGame();
	}
	if(libpuer_buf[6]=='1')printf("PS1 Bios Part 1.\n"),ofs=0,size=114688;
	else if(libpuer_buf[6]=='2')printf("PS1 Bios Part 2.\n"),ofs=114688,size=114688;
	else if(libpuer_buf[6]=='3')printf("PS1 Bios Part 3.\n"),ofs=229376,size=114688;
	else if(libpuer_buf[6]=='4')printf("PS1 Bios Part 4.\n"),ofs=344064,size=114688;
	else if(libpuer_buf[6]=='5')printf("PS1 Bios Part 5.\n"),ofs=458752,size=65536;
	else{
		fclose(f);fclose(mcr);
		printf("unknown part\n");
		for(;;sceKernelDelayThread(250000))if(!running)sceKernelExitGame();
	}
	fseek(mcr,0x4000+vmpofs,SEEK_SET);
	fseek(f,ofs,SEEK_SET);
	int readlen;
	for(;size;size-=readlen){
		readlen=fread(libpuer_buf,1,min(BUFLEN,size),mcr);
		fwrite(libpuer_buf,1,readlen,f);
	}
	fclose(f);
	fclose(mcr);
	sceIoRemove(memcard);
	printf("memory card file was deleted for next dumping.\n",memcard);
	printf("SCPHXXXXX.bin has been updated. Continue dumping or enjoy the bios.\n");
	sceKernelDelayThread(250000);
	sceKernelExitGame();
	return 0;
}
