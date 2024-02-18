#include "fs_module.h"

//#include "../qcommon.h"

#define __USE_BSD

#include <dirent.h>

#ifdef __linux__
#include <linux/limits.h>
#endif

#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

FS_GetHomeDirectoryFn FS_GetHomeDirectory;
struct fs_export_s fs_export_mod;
//FS_GetCacheDirectoryFn FS_GetCacheDirectory;
//FS_GetSecureDirectoryFn FS_GetSecureDirectory;
//FS_GetMediaDirectoryFn FS_GetMediaDirectory;
//FS_GetRuntimeDirectoryFn FS_GetRuntimeDirectory;

/*
* Sys_FS_GetHomeDirectory
*/
static const char *__Sys_FS_GetHomeDirectory( void )
{
	static char home[PATH_MAX] = { '\0' };

	if( home[0] == '\0' )
	{
#ifndef __ANDROID__
		const char *homeEnv = getenv( "HOME" );
		const char *base = NULL, *local = "";

#ifdef __MACOSX__
		base = homeEnv;
		local = "Library/Application Support/";
#else
		base = getenv( "XDG_DATA_HOME" );
		local = "";
		if( !base ) {
			base = homeEnv;
			local = ".local/share/";
		}
#endif

		if( base ) {
#ifdef __MACOSX__
			Q_snprintfz( home, sizeof( home ), "%s/%s%s-%d.%d", base, local, APPLICATION,
				APP_VERSION_MAJOR, APP_VERSION_MINOR );
#else
		 // Q_snprintfz( home, sizeof( home ), "%s/%s%c%s-%d.%d", base, local, tolower( *( (const char *)APPLICATION ) ),
		 // 	( (const char *)APPLICATION ) + 1, APP_VERSION_MAJOR, APP_VERSION_MINOR );
#endif
		}
#endif
	}

	if( home[0] == '\0' )
		return NULL;
	return home;
}


void FS_InitFSModule() {
  fs_export_mod.FS_GetHomeDirectory = __Sys_FS_GetHomeDirectory;

  FS_ImportFsModule(&fs_export_mod);
}

void FS_ImportFsModule(struct fs_export_s* module) {
	FS_GetHomeDirectory = module->FS_GetHomeDirectory;
}

//void FS_ImportFsModule(struct fs_export_s* module) {
//	FS_GetHomeDirectory = module->FS_GetHomeDirectory;
////	FS_GetCacheDirectory = module->FS_GetCacheDirectory;
////	FS_GetSecureDirectory = module->FS_GetSecureDirectory;
////	FS_GetMediaDirectory = module->FS_GetMediaDirectory;
////	FS_GetRuntimeDirectory = module->FS_GetRuntimeDirectory;
//}

