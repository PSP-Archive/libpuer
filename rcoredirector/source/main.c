#include "../../libpuer/libpuer.h"

PSP_MODULE_INFO("rcoredirector", PSP_MODULE_USER, 1, 1);
mainThreadParam("rcoredirector",8,128,PSP_THREAD_ATTR_USER);
STMOD_HANDLER previous = NULL;

static char rco_path[768],*rco_file;
SceUID sceIoOpenPatched(const char *file, int flags, SceMode mode){
	if(strlen(file)>13 && !strncasecmp(file,"flash0:/font/",13)){
		strcpy(rco_file,file+13);
		//debug("redirect %s to %s\n",file,rco_path);
		SceUID fd=sceIoOpen(rco_path, flags, mode);
		if(fd>=0){
			//debug("open ok\n");
			return fd;
		}
	}
	if(strlen(file)>21 && !strncasecmp(file,"flash0:/vsh/resource/",21)){
		strcpy(rco_file,file+21);
		debug("redirect %s to %s\n",file,rco_path);
		SceUID fd=sceIoOpen(rco_path, flags, mode);
		if(fd>=0){
			debug("open ok\n");
			return fd;
		}
	}
	return sceIoOpen(file, flags, mode);
}

int OnModuleStart(SceModule2 *mod){
	debug("%s\n",mod->modname);
	if(
		!strcmp(mod->modname, "scePaf_Module") ||
		//!strcmp(mod->modname, "sceVshCommonGui_Module") ||
#if 0
		!strcmp(mod->modname, "sceNpSignupPlugin_Module") ||
		!strcmp(mod->modname, "sceVshNpSignin_Module") ||
		!strcmp(mod->modname, "npadmin_plugin_module") ||
		!strcmp(mod->modname, "dd_helper_module") ||
		!strcmp(mod->modname, "video_plugin_module") ||
		!strcmp(mod->modname, "video_main_plugin_module") ||
		!strcmp(mod->modname, "msvideo_main_plugin_module") ||
		!strcmp(mod->modname, "sysconf_plugin_module") ||
		!strcmp(mod->modname, "sceVshGSPlugin_Module") ||
		!strcmp(mod->modname, "sceVshNetconf_Module") ||
		!strcmp(mod->modname, "recommend_main_module") ||
		!strcmp(mod->modname, "skype_plugin") ||
		!strcmp(mod->modname, "skype_main_plugin") ||
		!strcmp(mod->modname, "sceVshSubsPlugin_Module") ||
		!strcmp(mod->modname, "sceDialogmain_Module") ||
		!strcmp(mod->modname, "htmlviewer_plugin_module") ||
		!strcmp(mod->modname, "music_main_plugin_module") ||
		!strcmp(mod->modname, "oneseg_plugin_module") ||
		!strcmp(mod->modname, "photo_player_module") ||
		!strcmp(mod->modname, "music_player_module") ||
		!strcmp(mod->modname, "lftv_main_plugin_module") ||
		!strcmp(mod->modname, "game_plugin_module") ||
		!strcmp(mod->modname, "lftv_plugin_module") ||
		!strcmp(mod->modname, "opening_plugin_module") ||
		!strcmp(mod->modname, "launcher_plugin_module") ||
#endif
		!strcmp(mod->modname, "vsh_module") //maybe more?
	){
		void *addr = search_module_stub( mod , "IoFileMgrForUser" , 0x109F50BC);
		if(addr){
			REDIRECT_FUNCTION((u32)addr, sceIoOpenPatched);
			ClearCaches();
		}
	}

#if 1
	if(!strcmp(mod->modname, "opening_plugin_module")){
		strcpy(rco_file,"gameboot.pmf");
		SceUID fd=sceIoOpen(rco_path, PSP_O_RDONLY, 0777);
		if(fd>=0){ //make sure to kill loading flash0:/vsh/resource/gameboot.pmf again. thx plum.
			sceIoClose(fd);
			u32 text_addr = mod->text_addr;
			u32 text_end = mod->text_addr + mod->text_size;

			for(; text_addr < text_end; text_addr += 4){
				if(_lw(text_addr) == 0x00003021 && _lw(text_addr + 4) == 0x00003821){
					_sh(0, text_addr + 0xC);
					ClearCaches();
					break;
				}
			}
		}
	}
#endif
	return previous?previous(mod):0;
}

int prelude(){ //this is called from module_start directly. sctrlHENSetStartModuleHandler() must be called here, not in main().
	debug_waitpsplink();
	debug("prelude\n");
	strcpy(myfile,"rcoredirector.ini");
	ini_gets("rcoredirector", "rcopath", "ms0:/seplugins/rcoredirector/", rco_path, 768, mypath);
	if(*rco_path&&rco_path[strlen(rco_path)-1]!='/')strcat(rco_path,"/");
	debug("rco_path=%s\n",rco_path);
	rco_file=rco_path+strlen(rco_path);
	previous = sctrlHENSetStartModuleHandler(OnModuleStart);
	debug("%08x\n",previous);
	return 0;
}
