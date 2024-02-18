#ifndef __SYS_FS_MODULE_H
#define __SYS_FS_MODULE_H

#include "../../gameshared/q_arch.h"

//#define FS_READ				0
//#define FS_WRITE			1
//#define FS_APPEND			2
//#define FS_NOSIZE			0x80	// FS_NOSIZE bit tells that we're not interested in real size of the file
//									// it is merely a hint a proper file size may still be returned by FS_Open
//#define FS_RESERVED			0x100
//									
//#define FS_UPDATE			0x200
//#define FS_SECURE			0x400
//#define FS_CACHE			0x800
//
//#define FS_RWA_MASK			(FS_READ|FS_WRITE|FS_APPEND)
//
//#define FS_SEEK_CUR			0
//#define FS_SEEK_SET			1
//#define FS_SEEK_END			2
//
//enum fs_mediatype_e {
//	FS_MEDIA_IMAGES,
//
//	FS_MEDIA_NUM_TYPES
//};

#define DECLARE_TYPEDEF_METHOD( ret, name, ... ) \
	typedef ret(*name##Fn )( __VA_ARGS__ ); \
	ret name(__VA_ARGS__);

DECLARE_TYPEDEF_METHOD( const char *, FS_GetHomeDirectory, void );
//DECLARE_SYS_STUB_IMPL( const char *, FS_GetCacheDirectory, void );
//DECLARE_SYS_STUB_IMPL( const char *, FS_GetSecureDirectory, void );
//DECLARE_SYS_STUB_IMPL( const char *, FS_GetMediaDirectory, enum fs_mediatype_e type );
//DECLARE_SYS_STUB_IMPL( const char *, FS_GetRuntimeDirectory, void );

//DECLARE_SYS_STUB_IMPL( bool, FS_RemoveDirectory, const char *path );
//DECLARE_SYS_STUB_IMPL( bool, FS_CreateDirectory, const char *path );
//
//DECLARE_SYS_STUB_IMPL( const char *, FS_FindFirst, const char *path, unsigned musthave, unsigned canthave );
//DECLARE_SYS_STUB_IMPL( const char *, FS_FindNext, unsigned musthave, unsigned canthave );
//DECLARE_SYS_STUB_IMPL( void, FS_FindClose, void );
//
//DECLARE_SYS_STUB_IMPL( void *, FS_LockFile, const char *path );
//DECLARE_SYS_STUB_IMPL( void, FS_UnlockFile, void *handle );
//
//DECLARE_SYS_STUB_IMPL( time_t, FS_FileMTime, const char *filename );
//
//DECLARE_SYS_STUB_IMPL( FILE *, FS_fopen, const char *path, const char *mode );
//DECLARE_SYS_STUB_IMPL( int, FS_FileNo, FILE *fp );
//
//DECLARE_SYS_STUB_IMPL( void *, FS_MMapFile, int fileno, size_t size, size_t offset, void **mapping, size_t *mapping_offset );
//DECLARE_SYS_STUB_IMPL( void, FS_UnMMapFile, void *mapping, void *data, size_t size, size_t mapping_offset );
//
//DECLARE_SYS_STUB_IMPL( void, FS_AddFileToMedia, const char *filename );
//
//// virtual storage of pack files, such as .obb on Android
//DECLARE_SYS_STUB_IMPL( void, VFS_TouchGamePath, const char *gamedir, bool initial );
//DECLARE_SYS_STUB_IMPL( char **, VFS_ListFiles, const char *pattern, const char *prependBasePath, int *numFiles, bool listFiles, bool listDirs );
//DECLARE_SYS_STUB_IMPL( void *, VFS_FindFile, const char *filename );
//DECLARE_SYS_STUB_IMPL( const char *, VFS_VFSName, void *handle ); // must return null for null handle
//DECLARE_SYS_STUB_IMPL( unsigned, VFS_FileOffset, void *handle );  // ditto
//DECLARE_SYS_STUB_IMPL( unsigned, VFS_FileSize, void *handle );	  // ditto
//DECLARE_SYS_STUB_IMPL( void, VFS_Shutdown, void );

struct fs_export_s {
	FS_GetHomeDirectoryFn FS_GetHomeDirectory;
//	FS_GetCacheDirectoryFn FS_GetCacheDirectory;
//	FS_GetSecureDirectoryFn FS_GetSecureDirectory;
//	FS_GetMediaDirectoryFn FS_GetMediaDirectory;
//	FS_GetRuntimeDirectoryFn FS_GetRuntimeDirectory;
 // FS_RemoveDirectoryFn FS_RemoveDirectory;
 // FS_CreateDirectoryFn FS_CreateDirectory;
 // FS_FindFirstFn FS_FindFirst;
 // FS_FindNextFn FS_FindNext;
 // FS_FindCloseFn FS_FindClose;
 // FS_LockFileFn FS_LockFile;
 // FS_UnlockFileFn FS_UnlockFile;
 // FS_FileMTimeFn FS_FileMTime;
 // FS_fopenFn FS_fopen;
 // FS_FileNoFn FS_FileNo;
 // FS_MMapFileFn FS_MMapFile;
 // FS_UnMMapFileFn FS_UnMMapFile;
 // FS_AddFileToMediaFn FS_AddFileToMedia;
 // VFS_TouchGamePathFn VFS_TouchGamePath;
 // VFS_ListFilesFn VFS_ListFiles;
 // VFS_FindFileFn VFS_FindFile;
 // VFS_VFSNameFn VFS_VFSName;
 // VFS_FileOffsetFn VFS_FileOffset;
 // VFS_FileSizeFn VFS_FileSize;
 // VFS_ShutdownFn VFS_Shutdown;
};

void FS_InitFSModule();
void FS_ImportFsModule(struct fs_export_s* module);
//{
//	FS_GetHomeDirectory = module->FS_GetHomeDirectory;
//	FS_GetCacheDirectory = module->FS_GetCacheDirectory;
//	FS_GetSecureDirectory = module->FS_GetSecureDirectory;
//	FS_GetMediaDirectory = module->FS_GetMediaDirectory;
//	FS_GetRuntimeDirectory = module->FS_GetRuntimeDirectory;
//}

#endif
