/*
Copyright (C) 2002-2003 Victor Luchits

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/


// ---------------------------------------------------------------------------------------------------------------------------------
//
// Restrictions & freedoms pertaining to usage and redistribution of this software:
//
//  * This software is 100% free
//  * If you use this software (in part or in whole) you must credit the author.
//  * This software may not be re-distributed (in part or in whole) in a modified
//    form without clear documentation on how to obtain a copy of the original work.
//  * You may not use this software to directly or indirectly cause harm to others.
//  * This software is provided as-is and without warrantee. Use at your own risk.
//
// For more information, visit HTTP://www.FluidStudios.com
//
// ---------------------------------------------------------------------------------------------------------------------------------
// Originally created on 12/22/2000 by Paul Nettle
//
// Copyright 2000, Fluid Studios, Inc., all rights reserved.
// ---------------------------------------------------------------------------------------------------------------------------------

// Z_zone.c

#include "qcommon.h"
#include "qthreads.h"

//#define MEMTRASH

#define POOLNAMESIZE 128
#define MEMHEADER_SENTINEL1			0xDEADF00D
#define MEMHEADER_SENTINEL2			0xDF

#define MEMALIGNMENT_DEFAULT		16
#define MEM_TRACK_MEMORY 1

// ---------------------------------------------------------------------------------------------------------------------------------
// -DOC- Get to know these values. They represent the values that will be used to fill unused and deallocated RAM.
// ---------------------------------------------------------------------------------------------------------------------------------

static const bool randomWipe = false; 
static const bool alwaysWipeAll = false; 


static const unsigned int prefixPattern = 0xbaadf00d;      // Fill pattern for bytes preceeding allocated blocks
static const unsigned int postfixPattern = 0xdeadc0de;     // Fill pattern for bytes following allocated blocks
static const unsigned int unusedPattern = 0xfeedface;      // Fill pattern for freshly allocated blocks
static const unsigned int releasedPattern = 0xdeadbeef;    // Fill pattern for deallocated blocks

struct mempool_s
{
	const char *sourceFile;
	const char *sourceFunc;
	int sourceLine;

	size_t reportedSize; 
	size_t actualSize;
	
	// name of the pool
	const char* name;
	uint32_t flags;

	struct memAllocUnit_s *managed; // managed allocations to the pool
	struct memAllocUnit_s *temporary; // temporary allocations if not free this might indicated a leak

	struct mempool_s *next;
	struct mempool_s *parent;
};

struct memAllocUnit_s {
	void *actualAddress;
	void *reportedAddress;

	size_t offset;
	size_t alignment;
	size_t reportedSize; 
	size_t actualSize;

	const char *sourceFile;
	const char *sourceFunc;
	int sourceLine;

	// next link in the hash 
	struct memAllocUnit_s *hashNext;
	struct memAllocUnit_s *hashPrev;

	// pool this memheader belongs to
	struct memAllocPool_s* pool;
	struct memAllocUnit_s* next;
	struct memAllocUnit_s* prev;
};

//struct mempool_s
//{
//	// should always be MEMHEADER_SENTINEL1
//	unsigned int sentinel1;
//
//	// chain of individual memory allocations
//	struct memheader_s *chain;
//
//	// temporary, etc
//	int flags;
//
//	// total memory allocated in this pool (inside memheaders)
//	int totalsize;
//
//	// total memory allocated in this pool (actual malloc total)
//	int realsize;
//
//	// updated each time the pool is displayed by memlist, shows change from previous time (unless pool was freed)
//	int lastchecksize;
//
//	// name of the pool
//	char name[POOLNAMESIZE];
//
//	// linked into global mempool list or parent's children list
//	struct mempool_s *next;
//
//	struct mempool_s *parent;
//	struct mempool_s *child;
//
//	// file name and line where Mem_AllocPool was called
//	const char *filename;
//
//	int fileline;
//
//	// should always be MEMHEADER_SENTINEL1
//	unsigned int sentinel2;
//};

// ============================================================================

//#define SHOW_NONFREED

cvar_t *developerMemory;

static mempool_t *poolChain = NULL;

// used for temporary memory allocations around the engine, not for longterm
// storage, if anything in this pool stays allocated during gameplay, it is
// considered a leak
mempool_t *tempMemPool;

// only for zone
mempool_t *zoneMemPool;

static qmutex_t *memMutex;

static bool memory_initialized = false;
static bool commands_initialized = false;

#if defined( _DEBUG )
	static const size_t paddingSize = 4;
#else
	static const size_t paddingSize = 1;
#endif
static const size_t MaximumAlignment = sizeof( void * );

static struct mempool_s  *poolMemChain;

static struct mempool_s *reservoirMemPool;
static struct mempool_s  **reservoirMemPoolBuffer;
static size_t memPoolCount = 0;

static struct memAllocUnit_s *reservoirAllocUnit;
static struct memAllocUnit_s **reservoirAllocUnitBuffer;
static size_t memAllocCount = 0;

#define AllocHashSize ( 1u < 12u )
static struct memAllocUnit_s *hashAllocUnits[AllocHashSize];

static const char* insertCommas(unsigned int value)
{
	static char str[30];
	memset(str, 0, sizeof(str));

	sprintf(str, "%u", value);
	if (strlen(str) > 3)
	{
		memmove(&str[strlen(str) - 3], &str[strlen(str) - 4], 4);
		str[strlen(str) - 4] = ',';
	}
	if (strlen(str) > 7)
	{
		memmove(&str[strlen(str) - 7], &str[strlen(str) - 8], 8);
		str[strlen(str) - 8] = ',';
	}
	if (strlen(str) > 11)
	{
		memmove(&str[strlen(str) - 11], &str[strlen(str) - 12], 12);
		str[strlen(str) - 12] = ',';
	}

	return str;
}


static const char* __memorySizeString(uint32_t size)
{
	static char str[90];
	if (size > (1024 * 1024))
		printf(str, "%10s (%7.2fM)", insertCommas(size), ((float)size) / (1024.0f * 1024.0f));
	else if (size > 1024)
		sprintf(str, "%10s (%7.2fK)", insertCommas(size), ((float)size) / 1024.0f);
	else
		sprintf(str, "%10s bytes     ", insertCommas(size));
	return str;
}


static void __dumpAllocUnit(const struct memAllocUnit_s* allocUnit)
{
	Com_Printf("[I] Address (reported): %010p", allocUnit->reportedAddress);
	Com_Printf("[I] Address (actual)  : %010p", allocUnit->actualAddress);
	Com_Printf("[I] Size (reported)   : 0x%08X (%s)", (unsigned int)(allocUnit->reportedSize),
		__memorySizeString((unsigned int)(allocUnit->reportedSize)));
	Com_Printf("[I] Size (actual)     : 0x%08X (%s)", (unsigned int)(allocUnit->actualSize),
		__memorySizeString((unsigned int)(allocUnit->actualSize)));
	Com_Printf("[I] Owner             : %s(%d)::%s", allocUnit->sourceFile, allocUnit->sourceLine, allocUnit->sourceFunc);

//#if MMGR_BACKTRACE
//	Log("[I] %sBacktrace             : ", prefix);
//	dumpBacktrace(NULL, allocUnit);
//#endif

 // Log("[I] %sAllocation type   : %s", prefix, allocationTypes[allocUnit->allocationType]);
 // Log("[I] %sAllocation number : %d", prefix, allocUnit->allocationNumber);
}

static void __dumpMemoryPool(const struct mempool_s* allocPool) {
	Com_Printf("[P] Name              : %s", allocPool->name);
	Com_Printf("[P] Size (reported)   : 0x%08X (%s)", (unsigned int)(allocPool->reportedSize),
		__memorySizeString((unsigned int)(allocPool->reportedSize)));
	Com_Printf("[P] Size (actual)     : 0x%08X (%s)", (unsigned int)(allocPool->actualSize),
		__memorySizeString((unsigned int)(allocPool->actualSize)));
	Com_Printf("[P] Owner             : %s(%d)::%s", allocPool->sourceFile, allocPool->sourceLine, allocPool->sourceFunc);
}



bool __validateAllocUnit(const struct memAllocUnit_s* allocUnit)
{
	// Make sure the padding is untouched

	uint8_t* pre = ((uint8_t*)allocUnit->reportedAddress - paddingSize * sizeof(uint32_t));
	uint8_t* post = ((uint8_t*)allocUnit->reportedAddress + allocUnit->reportedSize);
	bool      errorFlag = false;
	const size_t paddingBytes = paddingSize * sizeof(uint32_t);
	for (size_t i = 0; i < paddingBytes; i++, pre++, post++)
	{
		const uint8_t expectedPrefixByte = (prefixPattern >> ((i % sizeof(uint32_t)) * 8)) & 0xFF;
		if (*pre != expectedPrefixByte)
		{
			Com_Printf("[!] A memory allocation unit was corrupt because of an underrun:");
			__dumpAllocUnit(allocUnit);
			errorFlag = true;
		}

		// If you hit this assert, then you should know that this allocation unit has been damaged. Something (possibly the
		// owner?) has underrun the allocation unit (modified a few bytes prior to the start). You can interrogate the
		// variable 'allocUnit' to see statistics and information about this damaged allocation unit.
		assert(*pre == expectedPrefixByte);

		const uint8_t expectedPostfixByte = (postfixPattern >> ((i % sizeof(uint32_t)) * 8)) & 0xFF;
		if (*post != expectedPostfixByte)
		{
			Com_Printf("[!] A memory allocation unit was corrupt because of an overrun:");
			__dumpAllocUnit(allocUnit);
			errorFlag = true;
		}

		// If you hit this assert, then you should know that this allocation unit has been damaged. Something (possibly the
		// owner?) has overrun the allocation unit (modified a few bytes after the end). You can interrogate the variable
		// 'allocUnit' to see statistics and information about this damaged allocation unit.
		assert(*post == expectedPostfixByte);
	}

	// Return the error status (we invert it, because a return of 'false' means error)

	return !errorFlag;
}


static void __wipeWithPattern(void* reportedAddress, size_t reportedSize, size_t originalReportedSize, uint32_t pattern)
{
	// For a serious test run, we use wipes of random a random value. However, if this causes a crash, we don't want it to
	// crash in a differnt place each time, so we specifically DO NOT call srand. If, by chance your program calls srand(),
	// you may wish to disable that when running with a random wipe test. This will make any crashes more consistent so they
	// can be tracked down easier.

	if (randomWipe)
	{
		pattern = ((rand() & 0xff) << 24) | ((rand() & 0xff) << 16) | ((rand() & 0xff) << 8) | (rand() & 0xff);
	}

	// -DOC- We should wipe with 0's if we're not in debug mode, so we can help hide bugs if possible when we release the
	// product. So uncomment the following line for releases.
	//
	// Note that the "alwaysWipeAll" should be turned on for this to have effect, otherwise it won't do much good. But we'll
	// leave it this way (as an option) because this does slow things down.
	//	pattern = 0;

	// This part of the operation is optional
	if (alwaysWipeAll && reportedSize > originalReportedSize)
	{
		// Fill the bulk

		uint32_t* lptr = (uint32_t*)(((char*)reportedAddress) + originalReportedSize);
		size_t length = reportedSize - originalReportedSize;
		for (size_t i = 0; i < (length >> 2); i++, lptr++)
		{
			*lptr = pattern;
		}

		// Fill the remainder

		unsigned int shiftCount = 0;
		char*        cptr = (char*)(lptr);
		for (size_t i = 0; i < (length & 0x3); i++, cptr++, shiftCount += 8)
		{
			*cptr = (char)((pattern & (0xff << shiftCount)) >> shiftCount);
		}
	}

	// Write in the prefix/postfix bytes

	// Calculate the correct start addresses for pre and post patterns relative to
	// allocUnit->reportedAddress, since it may have been offset due to alignment requirements
	uint8_t* pre = (uint8_t*)reportedAddress - paddingSize * sizeof(uint32_t);
	uint8_t* post = (uint8_t*)reportedAddress + reportedSize;

	const size_t paddingBytes = paddingSize * sizeof(uint32_t);
	for (size_t i = 0; i < paddingBytes; i++, pre++, post++)
	{
		*pre = (prefixPattern >> ((i % sizeof(uint32_t)) * 8)) & 0xFF;
		*post = (postfixPattern >> ((i % sizeof(uint32_t)) * 8)) & 0xFF;
	}
}



static inline size_t resolveUnitHashIndex(const void *reportedAddress ) {
	return ( ( (size_t)reportedAddress ) >> 4 ) & ( AllocHashSize - 1 );
}

static struct memAllocUnit_s *__findMemAllocUnit( const void *reportedAddress )
{
	// Just in case...
	assert( reportedAddress != NULL );

	// Use the address to locate the hash index. Note that we shift off the lower four bits. This is because most allocated
	// addresses will be on four-, eight- or even sixteen-byte boundaries. If we didn't do this, the hash index would not have
	// very good coverage.

	struct memAllocUnit_s *ptr = hashAllocUnits[resolveUnitHashIndex(reportedAddress)];
	while( ptr ) {
		if( ptr->reportedAddress == reportedAddress )
			return ptr;
		ptr = ptr->next;
	}
	return NULL;
}
static size_t __calculateActualSize(size_t reportedSize) {
	// We use DWORDS as our padding, and a uint32_t is guaranteed to be 4 bytes, but an int is not (ANSI defines an int as
	// being the standard word size for a processor; on a 32-bit machine, that's 4 bytes, but on a 64-bit machine, it's
	// 8 bytes, which means an int can actually be larger than a uint32_t.)
	
	return reportedSize + paddingSize * sizeof(uint32_t) * 2;
}

static void* __calculateReportedAddress(const void* actualAddress)
{
	return (void*)(((const char*)actualAddress) + sizeof(uint32_t) * paddingSize);
}

static struct memAllocUnit_s *__GetAllocUnitFromReserved()
{
	if(!reservoirAllocUnit) {
		reservoirAllocUnit = malloc( 256 * sizeof( struct memAllocUnit_s) );

		memset( reservoirAllocUnit, 0, sizeof( struct memAllocUnit_s ) * 256 );
		for (unsigned int i = 0; i < 256 - 1; i++)
		{
			reservoirAllocUnit[i].next = &reservoirAllocUnit[i + 1];
		}
		reservoirAllocUnitBuffer = realloc(reservoirAllocUnitBuffer, (memPoolCount  + 1) * sizeof(struct memAllocPool_s*));
		reservoirAllocUnitBuffer [memAllocCount++] = reservoirAllocUnit;
	}
	struct memAllocUnit_s *unit = reservoirAllocUnit;
	reservoirAllocUnit = unit->next;
	return unit;
}

static void __freeMemUnit(struct memAllocUnit_s* allocUnit) {
	free(allocUnit->reportedAddress);

	if(allocUnit->pool) {
		if(allocUnit->prev) 
			allocUnit->prev->next = allocUnit->next;
		if(allocUnit->next) 
			allocUnit->next->prev = allocUnit->prev;
		allocUnit->pool = NULL;
	}

	const size_t hashIndex = resolveUnitHashIndex(allocUnit);
	if(hashAllocUnits[hashIndex] == allocUnit) {
		hashAllocUnits[hashIndex] = hashAllocUnits[hashIndex]->hashNext; 
	} else {
		if(allocUnit->hashNext)
			allocUnit->hashNext->hashPrev = allocUnit->hashPrev;
		if(allocUnit->hashPrev)
			allocUnit->hashPrev->hashNext = allocUnit->hashNext;
	}

	allocUnit->next = reservoirAllocUnit; // we reserve next
	reservoirAllocUnit = allocUnit;
}

void *__Malloc_Mem( size_t alignment, size_t size,  const char *sourceFileName, int fileline, const char *sourceFuncName)
{
	QMutex_Lock( memMutex );
	struct memAllocUnit_s *unit = __GetAllocUnitFromReserved();
	unit->reportedSize = size;
	unit->alignment = alignment;
	
	unit->actualSize = __calculateActualSize( size ) + alignment;
	unit->actualAddress = malloc( unit->actualSize );
	unit->reportedAddress = __calculateReportedAddress( unit->actualAddress );
	unit->offset = ( (size_t)unit->reportedAddress ) % alignment;
	if( unit->offset ) {
		unit->reportedAddress = (uint8_t *)unit->reportedAddress + ( alignment - unit->offset  );
	}
	const size_t hashIndex = resolveUnitHashIndex( unit->reportedAddress );
	if( hashAllocUnits[hashIndex] ) {
		hashAllocUnits[hashIndex]->hashPrev = unit;
		unit->hashNext = hashAllocUnits[hashIndex];
	} 
	hashAllocUnits[hashIndex] = unit;
	QMutex_Unlock( memMutex );
	
	__wipeWithPattern( unit->reportedAddress, unit->reportedSize , 0, unusedPattern );

	return unit->reportedAddress;
}

void *__Realloc_Mem(void* mem, size_t reportedSize,  const char *sourceFilename, const char *sourceFuncName, int sourceFileline)
{
	QMutex_Lock( memMutex );
	struct memAllocUnit_s* unit = __findMemAllocUnit(mem);
	QMutex_Unlock( memMutex );
	
	assert(unit);
	assert(__validateAllocUnit(unit));

	const size_t updatedSize = __calculateActualSize( reportedSize ) + unit->alignment;
	void* actualAddress = malloc(updatedSize);
	void* newReportedAddress = actualAddress;
	size_t offset = ( (size_t)newReportedAddress ) % unit->alignment;
	if( offset ) {
		newReportedAddress  = (uint8_t *)unit->reportedAddress + ( unit->alignment - unit->offset  );
	}
	memcpy(newReportedAddress, unit->reportedAddress, min(unit->reportedSize, reportedSize));
	__wipeWithPattern(newReportedAddress, reportedSize, unit->reportedSize, unusedPattern); // wipe the new address 
	
	__wipeWithPattern(unit->reportedAddress, unit->reportedSize, 0, releasedPattern); // wipe the old address
	free(unit->actualAddress);
	
	unit->actualAddress = actualAddress;
	unit->reportedAddress = newReportedAddress;
	unit->reportedSize = reportedSize;
	unit->actualSize = updatedSize;

	unit->sourceLine = sourceFileline;
	unit->sourceFunc = sourceFuncName;
	unit->sourceFile = sourceFilename; 
	__wipeWithPattern( unit->reportedAddress, unit->reportedSize , 0, unusedPattern );
	
	return unit->reportedAddress;
}

void *__Pool_Malloc( struct mempool_s *pool, size_t alignment, size_t size, bool managed, const char *sourceFilename, const char *sourceFuncName, int sourceFileline )
{
	QMutex_Lock( memMutex );
	struct memAllocUnit_s *unit = __GetAllocUnitFromReserved();
	unit->reportedSize = size;
	unit->alignment = alignment;
	unit->actualSize = __calculateActualSize( size ) + alignment;
	unit->actualAddress = malloc( unit->actualSize );
	unit->reportedAddress = __calculateReportedAddress( unit->actualAddress );
	unit->offset = ( (size_t)unit->reportedAddress ) % alignment;
	if( unit->offset ) {
		unit->reportedAddress = (uint8_t *)unit->reportedAddress + ( alignment - unit->offset  );
	}
	const size_t hashIndex = resolveUnitHashIndex( unit->reportedAddress );
	if( hashAllocUnits[hashIndex] ) {
		hashAllocUnits[hashIndex]->hashPrev = unit;
		unit->hashNext = hashAllocUnits[hashIndex];
	} 
	hashAllocUnits[hashIndex] = unit;

	if(managed) {
		if(pool->managed) {
			pool->managed->prev = unit;
		}
		unit->next = pool->managed;	
		pool->managed = unit; 
	} else {
		if(pool->temporary) {
			pool->temporary->prev = unit;
		}
		unit->next = pool->temporary;	
		pool->temporary = unit; 
	}
	QMutex_Unlock( memMutex );
	
	__wipeWithPattern( unit->reportedAddress, unit->reportedSize , 0, unusedPattern );
	unit->sourceLine = sourceFileline;
	unit->sourceFunc = sourceFuncName;
	unit->sourceFile = sourceFilename ;
	return unit->reportedAddress;
}

void __Free_Mem(void* mem) {
	if( mem == NULL ) {
		return;
	}
	
	QMutex_Lock( memMutex );
	struct memAllocUnit_s* unit = __findMemAllocUnit(mem);	
	// if you hit this assertion then this is not memory tracked by mem 
	assert(unit);
	if(unit) {
		__freeMemUnit(unit);	
	}
	QMutex_Unlock( memMutex );
}

struct mempool_s *Pool_Create( struct mempool_s *parent, const char *name, int flags )
{
	QMutex_Lock( memMutex );
	if( !reservoirMemPool ) {
		reservoirMemPool = malloc( 256 * sizeof( struct mempool_s) );

		memset( reservoirMemPool, 0, sizeof( struct mempool_s ) * 256 );
		for( unsigned int i = 0; i < 256 - 1; i++ ) {
			reservoirMemPool[i].next = &reservoirMemPool[i + 1];
		}

		reservoirMemPoolBuffer = realloc( reservoirMemPoolBuffer, ( memPoolCount + 1 ) * sizeof( struct memAllocPool_s * ) );
		reservoirMemPoolBuffer[memPoolCount++] = reservoirMemPool;
	}

	struct mempool_s *pool = reservoirMemPool;
	reservoirMemPool = pool->next;

	memset( pool, 0, sizeof( struct mempool_s ) );
	pool->parent = parent;
	pool->name = name;
	pool->flags = flags;
	if( pool->parent ) {
		pool->next = parent->next;
		parent->next = pool;
	} else {
		pool->next = poolMemChain;
		poolMemChain = pool;
	}
	QMutex_Unlock( memMutex );
	return pool;
}

void Pool_Empty(struct mempool_s *pool) {
	QMutex_Lock( memMutex );
	if( pool->temporary ) {
		Com_Printf("[!] Alloc Units not correctly in temporary");
		__dumpMemoryPool( pool );
	}

	struct memAllocUnit_s* ptr;
	ptr = pool->managed;
	while(ptr) {
		__freeMemUnit(ptr);
		ptr = ptr->next;
	}
	
	ptr = pool->temporary;
	while(ptr) {
		__dumpAllocUnit(ptr);	
		__freeMemUnit(ptr);
		ptr = ptr->next;
	}
	QMutex_Unlock( memMutex );
}

void Pool_Free( struct mempool_s *pool )
{
	assert( pool );
	struct memAllocUnit_s *allocPtr;
	QMutex_Lock( memMutex );
	while( pool) {
		if( pool->temporary ) {
			Com_Printf( "[!] Alloc Units not correctly in temporary" );
			__dumpMemoryPool( pool );
		}

		allocPtr = pool->managed;
		while( allocPtr ) {
			__freeMemUnit( allocPtr );
			allocPtr = allocPtr->next;
		}

		allocPtr = pool->temporary;
		while( allocPtr ) {
			__dumpAllocUnit( allocPtr );
			__freeMemUnit( allocPtr );
			allocPtr = allocPtr->next;
		}
		pool->next = reservoirMemPool;
		reservoirMemPool = pool; // move it back to the reservoir for reused
		pool = pool->next;
	}
	QMutex_Unlock( memMutex );
}

void Pool_Assert_Flags(const struct mempool_s *pool, int requiredFlags) {
	assert(pool);
	assert((pool->flags & requiredFlags) == requiredFlags);
}

static void _Mem_Error( const char *format, ... )
{
	va_list	argptr;
	char msg[1024];

	va_start( argptr, format );
	Q_vsnprintfz( msg, sizeof( msg ), format, argptr );
	va_end( argptr );

	Sys_Error( msg );
}

void *_Mem_AllocExt( mempool_t *pool, size_t size, size_t alignment, int z, int musthave, int canthave, const char *filename, int fileline )
{
	void *base;
	size_t realsize;
	memheader_t *mem;

	if( size <= 0 )
		return NULL;

	// default to 16-bytes alignment
	if( !alignment )
		alignment = MEMALIGNMENT_DEFAULT;

	assert( pool != NULL );

	if( pool == NULL )
		_Mem_Error( "Mem_Alloc: pool == NULL (alloc at %s:%i)", filename, fileline );
	if( musthave && ( ( pool->flags & musthave ) != musthave ) )
		_Mem_Error( "Mem_Alloc: bad pool flags (musthave) (alloc at %s:%i)", filename, fileline );
	if( canthave && ( pool->flags & canthave ) )
		_Mem_Error( "Mem_Alloc: bad pool flags (canthave) (alloc at %s:%i)", filename, fileline );

	if( developerMemory && developerMemory->integer )
		Com_DPrintf( "Mem_Alloc: pool %s, file %s:%i, size %i bytes\n", pool->name, filename, fileline, size );

	QMutex_Lock( memMutex );

	pool->totalsize += size;
	realsize = sizeof( memheader_t ) + size + alignment + sizeof( int );

	pool->realsize += realsize;

	base = malloc( realsize );
	if( base == NULL )
		_Mem_Error( "Mem_Alloc: out of memory (alloc at %s:%i)", filename, fileline );

	// calculate address that aligns the end of the memheader_t to the specified alignment
	mem = ( memheader_t * )((((size_t)base + sizeof( memheader_t ) + (alignment-1)) & ~(alignment-1)) - sizeof( memheader_t ));
	mem->baseaddress = base;
	mem->filename = filename;
	mem->fileline = fileline;
	mem->size = size;
	mem->realsize = realsize;
	mem->pool = pool;
	mem->sentinel1 = MEMHEADER_SENTINEL1;

	// we have to use only a single byte for this sentinel, because it may not be aligned, and some platforms can't use unaligned accesses
	*( (uint8_t *) mem + sizeof( memheader_t ) + mem->size ) = MEMHEADER_SENTINEL2;

	// append to head of list
	mem->next = pool->chain;
	mem->prev = NULL;
	pool->chain = mem;
	if( mem->next )
		mem->next->prev = mem;

	QMutex_Unlock( memMutex );

	if( z )
		memset( (void *)( (uint8_t *) mem + sizeof( memheader_t ) ), 0, mem->size );

	return (void *)( (uint8_t *) mem + sizeof( memheader_t ) );
}

void *_Mem_Alloc( mempool_t *pool, size_t size, int musthave, int canthave, const char *filename, int fileline )
{
	return _Mem_AllocExt( pool, size, 0, 1, musthave, canthave, filename, fileline );
}

// FIXME: rewrite this?
void *_Mem_Realloc( void *data, size_t size, const char *filename, int fileline )
{
	void *newdata;
	memheader_t *mem;

	if( data == NULL )
		_Mem_Error( "Mem_Realloc: data == NULL (called at %s:%i)", filename, fileline );
	if( size <= 0 )
	{
		Mem_Free( data );
		return NULL;
	}

	mem = ( memheader_t * )( (uint8_t *) data - sizeof( memheader_t ) );
	if( size <= mem->size )
		return data;

	newdata = Mem_AllocExt( mem->pool, size, 0 );
	memcpy( newdata, data, mem->size );
	memset( (uint8_t *)newdata + mem->size, 0, size - mem->size );
	Mem_Free( data );

	return newdata;
}

char *_Mem_CopyString( mempool_t *pool, const char *in, const char *filename, int fileline )
{
	char *out;
	size_t num_chars = strlen( in ) + 1;
	size_t str_size = sizeof( char ) * num_chars;

	out = ( char* )_Mem_Alloc( pool, str_size, 0, 0, filename, fileline );
	memcpy( out, in, str_size );

	return out;
}

void _Mem_Free( void *data, int musthave, int canthave, const char *filename, int fileline )
{
	void *base;
	memheader_t *mem;
	mempool_t *pool;

	if( data == NULL )
		//_Mem_Error( "Mem_Free: data == NULL (called at %s:%i)", filename, fileline );
		return;

	mem = ( memheader_t * )( (uint8_t *) data - sizeof( memheader_t ) );

	assert( mem->sentinel1 == MEMHEADER_SENTINEL1 );
	assert( *( (uint8_t *) mem + sizeof( memheader_t ) + mem->size ) == MEMHEADER_SENTINEL2 );

	if( mem->sentinel1 != MEMHEADER_SENTINEL1 )
		_Mem_Error( "Mem_Free: trashed header sentinel 1 (alloc at %s:%i, free at %s:%i)", mem->filename, mem->fileline, filename, fileline );
	if( *( (uint8_t *)mem + sizeof( memheader_t ) + mem->size ) != MEMHEADER_SENTINEL2 )
		_Mem_Error( "Mem_Free: trashed header sentinel 2 (alloc at %s:%i, free at %s:%i)", mem->filename, mem->fileline, filename, fileline );

	pool = mem->pool;
	if( musthave && ( ( pool->flags & musthave ) != musthave ) )
		_Mem_Error( "Mem_Free: bad pool flags (musthave) (alloc at %s:%i)", filename, fileline );
	if( canthave && ( pool->flags & canthave ) )
		_Mem_Error( "Mem_Free: bad pool flags (canthave) (alloc at %s:%i)", filename, fileline );

	if( developerMemory && developerMemory->integer )
		Com_DPrintf( "Mem_Free: pool %s, alloc %s:%i, free %s:%i, size %i bytes\n", pool->name, mem->filename, mem->fileline, filename, fileline, mem->size );

	QMutex_Lock( memMutex );

	// unlink memheader from doubly linked list
	if( ( mem->prev ? mem->prev->next != mem : pool->chain != mem ) || ( mem->next && mem->next->prev != mem ) )
		_Mem_Error( "Mem_Free: not allocated or double freed (free at %s:%i)", filename, fileline );

	if( mem->prev )
		mem->prev->next = mem->next;
	else
		pool->chain = mem->next;
	if( mem->next )
		mem->next->prev = mem->prev;

	// memheader has been unlinked, do the actual free now
	pool->totalsize -= mem->size;

	base = mem->baseaddress;
	pool->realsize -= mem->realsize;

	QMutex_Unlock( memMutex );

#ifdef MEMTRASH
	memset( mem, 0xBF, sizeof( memheader_t ) + mem->size + sizeof( int ) );
#endif

	free( base );
}

mempool_t *_Mem_AllocPool( mempool_t *parent, const char *name, int flags, const char *filename, int fileline )
{
	mempool_t *pool;

	if( parent && ( parent->flags & MEMPOOL_TEMPORARY ) )
		_Mem_Error( "Mem_AllocPool: nested temporary pools are not allowed (allocpool at %s:%i)", filename, fileline );
	if( flags & MEMPOOL_TEMPORARY )
		_Mem_Error( "Mem_AllocPool: tried to allocate temporary pool, use Mem_AllocTempPool instead (allocpool at %s:%i)", filename, fileline );

	pool = ( mempool_t* )malloc( sizeof( mempool_t ) );
	if( pool == NULL )
		_Mem_Error( "Mem_AllocPool: out of memory (allocpool at %s:%i)", filename, fileline );

	memset( pool, 0, sizeof( mempool_t ) );
	pool->sentinel1 = MEMHEADER_SENTINEL1;
	pool->sentinel2 = MEMHEADER_SENTINEL1;
	pool->filename = filename;
	pool->fileline = fileline;
	pool->flags = flags;
	pool->chain = NULL;
	pool->parent = parent;
	pool->child = NULL;
	pool->totalsize = 0;
	pool->realsize = sizeof( mempool_t );
	Q_strncpyz( pool->name, name, sizeof( pool->name ) );

	if( parent )
	{
		pool->next = parent->child;
		parent->child = pool;
	}
	else
	{
		pool->next = poolChain;
		poolChain = pool;
	}

	return pool;
}

mempool_t *_Mem_AllocTempPool( const char *name, const char *filename, int fileline )
{
	mempool_t *pool;

	pool = _Mem_AllocPool( NULL, name, 0, filename, fileline );
	pool->flags = MEMPOOL_TEMPORARY;

	return pool;
}

void _Mem_FreePool( mempool_t **pool, int musthave, int canthave, const char *filename, int fileline )
{
	mempool_t **chainAddress;
#ifdef SHOW_NONFREED
	memheader_t *mem;
#endif

	if( !( *pool ) )
		return;
	if( musthave && ( ( ( *pool )->flags & musthave ) != musthave ) )
		_Mem_Error( "Mem_FreePool: bad pool flags (musthave) (alloc at %s:%i)", filename, fileline );
	if( canthave && ( ( *pool )->flags & canthave ) )
		_Mem_Error( "Mem_FreePool: bad pool flags (canthave) (alloc at %s:%i)", filename, fileline );

	// recurse into children
	// note that children will be freed no matter if their flags
	// do not match musthave\canthave pair
	while( ( *pool )->child ) {
		mempool_t *tmp = ( *pool )->child;
		_Mem_FreePool( &tmp, 0, 0, filename, fileline );
	}

	assert( ( *pool )->sentinel1 == MEMHEADER_SENTINEL1 );
	assert( ( *pool )->sentinel2 == MEMHEADER_SENTINEL1 );

	if( ( *pool )->sentinel1 != MEMHEADER_SENTINEL1 )
		_Mem_Error( "Mem_FreePool: trashed pool sentinel 1 (allocpool at %s:%i, freepool at %s:%i)", ( *pool )->filename, ( *pool )->fileline, filename, fileline );
	if( ( *pool )->sentinel2 != MEMHEADER_SENTINEL1 )
		_Mem_Error( "Mem_FreePool: trashed pool sentinel 2 (allocpool at %s:%i, freepool at %s:%i)", ( *pool )->filename, ( *pool )->fileline, filename, fileline );

#ifdef SHOW_NONFREED
	if( ( *pool )->chain )
		Com_Printf( "Warning: Memory pool %s has resources that weren't freed:\n", ( *pool )->name );
	for( mem = ( *pool )->chain; mem; mem = mem->next )
	{
		Com_Printf( "%10i bytes allocated at %s:%i\n", mem->size, mem->filename, mem->fileline );
	}
#endif

	// unlink pool from chain
	if( ( *pool )->parent )
		for( chainAddress = &( *pool )->parent->child; *chainAddress && *chainAddress != *pool; chainAddress = &( ( *chainAddress )->next ) ) ;
	else
		for( chainAddress = &poolChain; *chainAddress && *chainAddress != *pool; chainAddress = &( ( *chainAddress )->next ) ) ;

	if( *chainAddress != *pool )
		_Mem_Error( "Mem_FreePool: pool already free (freepool at %s:%i)", filename, fileline );

	while( ( *pool )->chain )  // free memory owned by the pool
		Mem_Free( (void *)( (uint8_t *)( *pool )->chain + sizeof( memheader_t ) ) );

	*chainAddress = ( *pool )->next;

	// free the pool itself
#ifdef MEMTRASH
	memset( *pool, 0xBF, sizeof( mempool_t ) );
#endif
	free( *pool );
	*pool = NULL;
}

void _Mem_EmptyPool( mempool_t *pool, int musthave, int canthave, const char *filename, int fileline )
{
	mempool_t *child, *next;
#ifdef SHOW_NONFREED
	memheader_t *mem;
#endif

	if( pool == NULL )
		_Mem_Error( "Mem_EmptyPool: pool == NULL (emptypool at %s:%i)", filename, fileline );
	if( musthave && ( ( pool->flags & musthave ) != musthave ) )
		_Mem_Error( "Mem_EmptyPool: bad pool flags (musthave) (alloc at %s:%i)", filename, fileline );
	if( canthave && ( pool->flags & canthave ) )
		_Mem_Error( "Mem_EmptyPool: bad pool flags (canthave) (alloc at %s:%i)", filename, fileline );

	// recurse into children
	if( pool->child )
	{
		for( child = pool->child; child; child = next )
		{
			next = child->next;
			_Mem_EmptyPool( child, 0, 0, filename, fileline );
		}
	}

	assert( pool->sentinel1 == MEMHEADER_SENTINEL1 );
	assert( pool->sentinel2 == MEMHEADER_SENTINEL1 );

	if( pool->sentinel1 != MEMHEADER_SENTINEL1 )
		_Mem_Error( "Mem_EmptyPool: trashed pool sentinel 1 (allocpool at %s:%i, emptypool at %s:%i)", pool->filename, pool->fileline, filename, fileline );
	if( pool->sentinel2 != MEMHEADER_SENTINEL1 )
		_Mem_Error( "Mem_EmptyPool: trashed pool sentinel 2 (allocpool at %s:%i, emptypool at %s:%i)", pool->filename, pool->fileline, filename, fileline );

#ifdef SHOW_NONFREED
	if( pool->chain )
		Com_Printf( "Warning: Memory pool %s has resources that weren't freed:\n", pool->name );
	for( mem = pool->chain; mem; mem = mem->next )
	{
		Com_Printf( "%10i bytes allocated at %s:%i\n", mem->size, mem->filename, mem->fileline );
	}
#endif
	while( pool->chain )        // free memory owned by the pool
		Mem_Free( (void *)( (uint8_t *) pool->chain + sizeof( memheader_t ) ) );
}

size_t Mem_PoolTotalSize( mempool_t *pool )
{
	assert( pool != NULL );

	return pool->totalsize;
}

void _Mem_CheckSentinels( void *data, const char *filename, int fileline )
{
 // memheader_t *mem;

 // if( data == NULL )
 // 	_Mem_Error( "Mem_CheckSentinels: data == NULL (sentinel check at %s:%i)", filename, fileline );

 // mem = (memheader_t *)( (uint8_t *) data - sizeof( memheader_t ) );

 // assert( mem->sentinel1 == MEMHEADER_SENTINEL1 );
 // assert( *( (uint8_t *) mem + sizeof( memheader_t ) + mem->size ) == MEMHEADER_SENTINEL2 );

 // if( mem->sentinel1 != MEMHEADER_SENTINEL1 )
 // 	_Mem_Error( "Mem_CheckSentinels: trashed header sentinel 1 (block allocated at %s:%i, sentinel check at %s:%i)", mem->filename, mem->fileline, filename, fileline );
 // if( *( (uint8_t *) mem + sizeof( memheader_t ) + mem->size ) != MEMHEADER_SENTINEL2 )
 // 	_Mem_Error( "Mem_CheckSentinels: trashed header sentinel 2 (block allocated at %s:%i, sentinel check at %s:%i)", mem->filename, mem->fileline, filename, fileline );
}

static void _Mem_CheckSentinelsPool( mempool_t *pool, const char *filename, int fileline )
{
 // memheader_t *mem;
 // mempool_t *child;

 // // recurse into children
 // if( pool->child )
 // {
 // 	for( child = pool->child; child; child = child->next )
 // 		_Mem_CheckSentinelsPool( child, filename, fileline );
 // }

 // assert( pool->sentinel1 == MEMHEADER_SENTINEL1 );
 // assert( pool->sentinel2 == MEMHEADER_SENTINEL1 );

 // if( pool->sentinel1 != MEMHEADER_SENTINEL1 )
 // 	_Mem_Error( "_Mem_CheckSentinelsPool: trashed pool sentinel 1 (allocpool at %s:%i, sentinel check at %s:%i)", pool->filename, pool->fileline, filename, fileline );
 // if( pool->sentinel2 != MEMHEADER_SENTINEL1 )
 // 	_Mem_Error( "_Mem_CheckSentinelsPool: trashed pool sentinel 2 (allocpool at %s:%i, sentinel check at %s:%i)", pool->filename, pool->fileline, filename, fileline );

 // for( mem = pool->chain; mem; mem = mem->next )
 // 	_Mem_CheckSentinels( (void *)( (uint8_t *) mem + sizeof( memheader_t ) ), filename, fileline );
}

void _Mem_CheckSentinelsGlobal( const char *filename, int fileline )
{
 // mempool_t *pool;

 // for( pool = poolChain; pool; pool = pool->next )
 // 	_Mem_CheckSentinelsPool( pool, filename, fileline );
}

static void Mem_CountPoolStats( mempool_t *pool, int *count, int *size, int *realsize )
{
	mempool_t *child;

	// recurse into children
	if( pool->child )
	{
		for( child = pool->child; child; child = child->next )
			Mem_CountPoolStats( child, count, size, realsize );
	}

	if( count )
		( *count )++;
	if( size )
		( *size ) += pool->totalsize;
	if( realsize )
		( *realsize ) += pool->realsize;
}

static void Mem_PrintStats( void )
{
	int count, size, real;
	int total, totalsize, realsize;
	mempool_t *pool;
	memheader_t *mem;

	Mem_CheckSentinelsGlobal();

	for( total = 0, totalsize = 0, realsize = 0, pool = poolChain; pool; pool = pool->next )
	{
		count = 0; size = 0; real = 0;
		Mem_CountPoolStats( pool, &count, &size, &real );
		total += count; totalsize += size; realsize += real;
	}

	Com_Printf( "%i memory pools, totalling %i bytes (%.3fMB), %i bytes (%.3fMB) actual\n", total, totalsize, totalsize / 1048576.0,
		realsize, realsize / 1048576.0 );

	// temporary pools are not nested
	for( pool = poolChain; pool; pool = pool->next )
	{
		if( ( pool->flags & MEMPOOL_TEMPORARY ) && pool->chain )
		{
			Com_Printf( "%i bytes (%.3fMB) (%i bytes (%.3fMB actual)) of temporary memory still allocated (Leak!)\n", pool->totalsize, pool->totalsize / 1048576.0,
				pool->realsize, pool->realsize / 1048576.0 );
			Com_Printf( "listing temporary memory allocations for %s:\n", pool->name );

			for( mem = tempMemPool->chain; mem; mem = mem->next )
				Com_Printf( "%10i bytes allocated at %s:%i\n", mem->size, mem->filename, mem->fileline );
		}
	}
}

static void Mem_PrintPoolStats( mempool_t *pool, int listchildren, int listallocations )
{
	mempool_t *child;
	memheader_t *mem;
	int totalsize = 0, realsize = 0;

	Mem_CountPoolStats( pool, NULL, &totalsize, &realsize );

	if( pool->parent )
	{
		if( pool->lastchecksize != 0 && totalsize != pool->lastchecksize )
			Com_Printf( "%6ik (%6ik actual) %s:%s (%i byte change)\n", ( totalsize + 1023 ) / 1024, ( realsize + 1023 ) / 1024, pool->parent->name, pool->name, totalsize - pool->lastchecksize );
		else
			Com_Printf( "%6ik (%6ik actual) %s:%s\n", ( totalsize + 1023 ) / 1024, ( realsize + 1023 ) / 1024, pool->parent->name, pool->name );
	}
	else
	{
		if( pool->lastchecksize != 0 && totalsize != pool->lastchecksize )
			Com_Printf( "%6ik (%6ik actual) %s (%i byte change)\n", ( totalsize + 1023 ) / 1024, ( realsize + 1023 ) / 1024, pool->name, totalsize - pool->lastchecksize );
		else
			Com_Printf( "%6ik (%6ik actual) %s\n", ( totalsize + 1023 ) / 1024, ( realsize + 1023 ) / 1024, pool->name );
	}

	pool->lastchecksize = totalsize;

	if( listallocations )
	{
		for( mem = pool->chain; mem; mem = mem->next )
			Com_Printf( "%10i bytes allocated at %s:%i\n", mem->size, mem->filename, mem->fileline );
	}

	if( listchildren )
	{
		if( pool->child )
		{
			for( child = pool->child; child; child = child->next )
				Mem_PrintPoolStats( child, listchildren, listallocations );
		}
	}
}

static void Mem_PrintList( int listchildren, int listallocations )
{
	mempool_t *pool;

	Mem_CheckSentinelsGlobal();

	Com_Printf( "memory pool list:\n" "size    name\n" );

	for( pool = poolChain; pool; pool = pool->next )
		Mem_PrintPoolStats( pool, listchildren, listallocations );
}

static void MemList_f( void )
{
	mempool_t *pool;
	const char *name = "";

	switch( Cmd_Argc() )
	{
	case 1:
		Mem_PrintList( true, false );
		Mem_PrintStats();
		return;
	case 2:
		if( !Q_stricmp( Cmd_Argv( 1 ), "all" ) )
		{
			Mem_PrintList( true, true );
			Mem_PrintStats();
			break;
		}
		name = Cmd_Argv( 1 );
		break;
	default:
		name = Cmd_Args();
		break;
	}

	for( pool = poolChain; pool; pool = pool->next )
	{
		if( !Q_stricmp( pool->name, name ) )
		{
			Com_Printf( "memory pool list:\n" "size    name\n" );
			Mem_PrintPoolStats( pool, true, true );
			return;
		}
	}

	Com_Printf( "MemList_f: unknown pool name '%s'. Usage: [all|pool]\n", name, Cmd_Argv( 0 ) );
}

static void MemStats_f( void )
{
	Mem_CheckSentinelsGlobal();
	Mem_PrintStats();
}


/*
* Memory_Init
*/
void Memory_Init( void )
{
	assert( !memory_initialized );

	memMutex = QMutex_Create();

	zoneMemPool = Mem_AllocPool( NULL, "Zone" );
	tempMemPool = Mem_AllocTempPool( "Temporary Memory" );

	memory_initialized = true;
}

mempool_t *Mem_DefaultTempPool()
{
	return tempMemPool;
}

mempool_t *Mem_DefaultZonePool()
{
	return zoneMemPool;
}

/*
* Memory_InitCommands
*/
void Memory_InitCommands( void )
{
	assert( !commands_initialized );

	developerMemory = Cvar_Get( "developerMemory", "0", 0 );

	Cmd_AddCommand( "memlist", MemList_f );
	Cmd_AddCommand( "memstats", MemStats_f );

	commands_initialized = true;
}

/*
* Memory_Shutdown
* 
* NOTE: Should be the last called function before shutdown!
*/
void Memory_Shutdown( void )
{
	mempool_t *pool, *next;

	if( !memory_initialized )
		return;

	// set the cvar to NULL so nothing is printed to non-existing console
	developerMemory = NULL;

	Mem_CheckSentinelsGlobal();

	Mem_FreePool( &zoneMemPool );
	Mem_FreePool( &tempMemPool );

	for( pool = poolChain; pool; pool = next )
	{
		// do it here, because pool is to be freed
		// and the chain will be broken
		next = pool->next;
#ifdef SHOW_NONFREED
		Com_Printf( "Warning: Memory pool %s was never freed\n", pool->name );
#endif
		Mem_FreePool( &pool );
	}

	QMutex_Destroy( &memMutex );

	memory_initialized = false;
}

/*
* Memory_ShutdownCommands
*/
void Memory_ShutdownCommands( void )
{
	if( !commands_initialized )
		return;

	Cmd_RemoveCommand( "memlist" );
	Cmd_RemoveCommand( "memstats" );
}
