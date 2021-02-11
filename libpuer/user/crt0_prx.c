/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * crt0_prx.c - Pure PRX startup code.
 *
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 *
 * $Id: crt0.c 1526 2005-12-06 21:56:06Z tyranid $
 */

#include <pspkerneltypes.h>
#include <pspmoduleinfo.h>
#include <pspthreadman.h>
#include <stdlib.h>
#include <string.h>

/* Default thread parameters for the main program thread. */
#define DEFAULT_THREAD_PRIORITY 32
#define DEFAULT_THREAD_ATTRIBUTE 0
#define DEFAULT_THREAD_STACK_KB_SIZE 256
#define DEFAULT_MAIN_THREAD_NAME "user_main"

/* Define us as a prx */
int __pspsdk_is_prx = 1;

/* If these variables are defined by the program, then they override the
   defaults given above. */
extern int sce_newlib_nocreate_thread_in_start __attribute__((weak));
extern unsigned int sce_newlib_priority __attribute__((weak));
extern unsigned int sce_newlib_attribute __attribute__((weak));
extern unsigned int sce_newlib_stack_kb_size __attribute__((weak));
extern const char*  sce_newlib_main_thread_name __attribute__((weak));

/* This is declared weak in case someone compiles an empty program.  That
   program won't work on the PSP, but it could be useful for testing the
   toolchain. */
extern SceModuleInfo module_info __attribute__((weak));

/* Allow newlib/psplibc to provide an init hook to be called before main */
extern void __psp_libc_init(int argc, char *argv[]) __attribute__((weak));

extern void _init();
extern void _fini();
extern void __psp_fdman_init();
extern void libpuer_startup();

extern int prelude(int argc, char *argv[]) __attribute__((weak));
extern int main(int argc, char *argv[]) __attribute__((weak));
extern int finale(int argc, char *argv[]) __attribute__((weak));
extern int main_returns_immediately __attribute__((weak));

/* The maximum number of arguments that can be passed to main(). */
#define ARG_MAX 19
char *argv[ARG_MAX + 1];
int argc;

static int finished=0;
void _fini2(){
	if(finished)return;
	_fini();
	finished=1;
}

/**
 * Main program thread
 *
 * Initializes runtime parameters and calls the program's main().
 *
 * @param args - Size (in bytes) of the argp parameter.
 * @param argp - Pointer to program arguments.  Each argument is a NUL-terminated string.
 */
void _main(SceSize args, void *argp)
{
	/* Make sure _fini() is called when the program ends. */
	//atexit(_fini2);

	/* Call main(). */
	int res = main(argc, argv);

	/* Return control to the operating system. */
	if(&main_returns_immediately == NULL)exit(res);
}

//int module_start(SceSize args, void *argp) __attribute__((alias("_start")));

/**
 * Startup thread
 *
 * Creates the main program thread based on variables defined by the program.
 *
 * @param args - Size (in bytes) of arguments passed to the program by the kernel.
 * @param argp - Pointer to arguments passed by the kernel.
 */
int module_start(SceSize args, void *argp){
	argc=0;finished=0;

	int loc = 0;
	char *ptr = argp;

	_init();

	/* Turn our thread arguments into main()'s argc and argv[]. */
	while(loc < args)
	{
		argv[argc] = &ptr[loc];
		loc += strlen(&ptr[loc]) + 1;
		argc++;
		if(argc == ARG_MAX-1)
		{
			break;
		}
	}
	argv[argc] = NULL;

	/* Call libc initialization hook */
	//if(__psp_libc_init != NULL)
	//	__psp_libc_init(argc, argv);
	__psp_fdman_init();

#if 0
	if (&sce_newlib_nocreate_thread_in_start != NULL) {
		/* The program does not want main() to be run in a seperate thread. */
		_main(args, argp);
		return 1;
	}
#endif

	libpuer_startup();
	if(prelude)prelude(argc, argv); //register callback here, if required.

	int priority = DEFAULT_THREAD_PRIORITY;
	unsigned int attribute = DEFAULT_THREAD_ATTRIBUTE;
	unsigned int stackSize = DEFAULT_THREAD_STACK_KB_SIZE * 1024;
	const char *threadName = DEFAULT_MAIN_THREAD_NAME;

	if(&sce_newlib_priority != NULL)priority = sce_newlib_priority;
	if(&sce_newlib_attribute != NULL)attribute = sce_newlib_attribute;
	if(&sce_newlib_stack_kb_size != NULL)stackSize = sce_newlib_stack_kb_size * 1024;
	if(&sce_newlib_main_thread_name != NULL)threadName = sce_newlib_main_thread_name;

	atexit(_fini2);
	if(main){
		SceUID thid;
		thid = sceKernelCreateThread(threadName, (void *) _main, priority, stackSize, attribute, 0);
		sceKernelStartThread(thid, args, argp);
	}

	return 0;
}

int module_stop( SceSize args, void *argp ){
	if(finale)finale(argc, argv); //perhaps argv shouldn't be used.
	if(&main_returns_immediately != NULL)exit(0);
	_fini2(); //just to make sure...
	return 0;
}
