#ifdef SCTRL

// based on popsbridge_v3_src
#include "../libpuer.h"

void* search_module_export(SceModule2 *pMod, const char *szLib, u32 nid)
{
	struct SceLibraryEntryTable *entry;
	void *entTab = pMod->ent_top;
	int entLen = pMod->ent_size;
	int i = 0;

	while(i < entLen) {
		int count;
		int total;
		unsigned int *vars;
		entry = (struct SceLibraryEntryTable *) (entTab + i);

		if( (entry->libname == szLib) || (entry->libname && strcmp(entry->libname, szLib) == 0)) {
			total = entry->stubcount + entry->vstubcount;		
			vars = entry->entrytable;

			if(total > 0) {
				for(count = 0; count < total ; count++) {
					if (vars[count] == nid) 
					{
						return (void *)(vars[count+total]);
					}
				}
			}
		}

		i += (entry->len * 4);
	}
	return NULL;
}

void* search_module_stub(SceModule2 *pMod, const char *szLib, u32 nid)
{
	void *entTab = pMod ->stub_top;
	int entLen = pMod->stub_size;
	struct SceLibraryStubTable *current;
	int i = 0 ,j;

	while( i < entLen ) {
		current = (struct SceLibraryStubTable *)(entTab + i);
		if(strcmp(current->libname, szLib ) == 0) {
			for(j=0;j< current->stubcount ;j++) {
				if( current->nidtable[j] == nid ) {
					return (void *)((u32)(current->stubtable) + 8*j );
				}
			}

			break;
		}
		i += (current->len * 4);
	}

	return NULL;
}

#endif
