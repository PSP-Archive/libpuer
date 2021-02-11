#include "../libpuer.h"

static char eboot_id[256]; //bah
const char *get_eboot_id(const char *eboot_name){
	char head[16];
	SceUID fd=sceIoOpen(eboot_name,PSP_O_RDONLY,0777);
	sceIoRead(fd,head,16);
	if(memcmp(head,"\0PBP",4)){//||read32(head+4)!=0x00010000){ //LittleBigPlanet uses 0x00010001... shouldn't check lol
		sceIoClose(fd);
		goto end;
	}
	int param_offset=read32(head+8);
	int param_size=read32(head+12)-param_offset;
#ifdef PSP
	SceUID uid=sceKernelAllocPartitionMemory(2,"EBOOTReader",PSP_SMEM_Low,param_size,NULL);
	if(uid<0){
		sceIoClose(fd);
		goto end;
	}
	char *p=sceKernelGetBlockHeadAddr(uid);
#else
	char *p=malloc(param_size);
	if(!p){
		sceIoClose(fd);
		goto end;
	}
#endif
	sceIoLseek(fd,param_offset,SEEK_SET);
	sceIoRead(fd,p,param_size);
	sceIoClose(fd);
	if(memcmp(p,"\0PSF",4)||read32(p+4)!=0x00000101){
#ifdef PSP
		sceKernelFreePartitionMemory(uid);
#else
		free(p);
#endif
		goto end;
	}
	int label_offset=read32(p+8);
	int data_offset=read32(p+12);
	int nlabel=read32(p+16);
	int i=0;
	for(;i<nlabel;i++){
		if(!strcmp(p+label_offset+read16(p+20+16*i),"DISC_ID")){ //seems to be 16bytes long
			int datasize=read32(p+20+16*i+8);
			//if(datasize>19)datasize=19;
			memcpy(eboot_id,p+data_offset+read32(p+20+16*i+12),datasize);
			eboot_id[datasize]=0;
			break;
		}
	}
#ifdef PSP
	sceKernelFreePartitionMemory(uid);
#else
	free(p);
#endif
	if(i==nlabel)return NULL;
	return eboot_id;
end:
	return NULL;
}
