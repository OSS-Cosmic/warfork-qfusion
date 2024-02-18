#include "fs_module.h"

FS_GetHomeDirectoryFn FS_GetHomeDirectory;
struct fs_export_s fs_export_mod;

//FS_GetCacheDirectoryFn FS_GetCacheDirectory;
//FS_GetSecureDirectoryFn FS_GetSecureDirectory;
//FS_GetMediaDirectoryFn FS_GetMediaDirectory;
//FS_GetRuntimeDirectoryFn FS_GetRuntimeDirectory;

void FS_InitFSModule() {
	assert(0);
}

void FS_ImportFsModule(struct fs_export_s* module) {
	FS_GetHomeDirectory = module->FS_GetHomeDirectory;
}

//	FS_GetCacheDirectory = module->FS_GetCacheDirectory;
//	FS_GetSecureDirectory = module->FS_GetSecureDirectory;
//	FS_GetMediaDirectory = module->FS_GetMediaDirectory;
//	FS_GetRuntimeDirectory = module->FS_GetRuntimeDirectory;
