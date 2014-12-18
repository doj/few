/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:
 */
/* Copyright (c) 2004..2008 Dirk Jagdmann <doj@cubic.org>

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you
   must not claim that you wrote the original software. If you use
   this software in a product, an acknowledgment in the product
   documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and
   must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution. */

// TODO:
// - filelen on Win32
// - readwrite support for Win32
// - split this file into unix/ and win/ directory

#if defined(__APPLE__) && !defined(__unix__)
#define __unix__ 1
#endif

#ifdef __unix__
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#endif

#ifdef _WIN32
#pragma warning( disable : 4786 )
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <iostream>
#include <map>

#include "memorymap.h"

using namespace std;

namespace doj
{

#ifdef __unix__
    /**
       data needed to handle a memory map on linux systems.
    */
    struct mmap_info {
	int fh;
	uint64_t filelen;
	string filename;
	bool readonly;
    };
#endif

#ifdef _WIN32
    /**
       data needed to handle a memory map on WIN32 systems.
    */
    struct mmap_info {
	HANDLE file;
	HANDLE map;
	uint64_t filelen;
	string filename;
    };
#endif

    typedef map<void*, struct mmap_info> memorymap_registry_t; ///< registry type

    /**
       the memorymap registry contains all active memory maps and the
       needed data. This information is needed for unmapping and to
       retrieve the map size via the memorymapsize(...) function.
    */
    static memorymap_registry_t memorymap_registry;

#ifdef __unix__

    //lint -esym(952,filename)
    void* memorymap(const char* filename, const bool readonly_)
    {
	struct mmap_info info;
	info.filename=filename;
	info.readonly=readonly_;

	// get file size
	struct stat buf;
	if(stat(filename, &buf) < 0)
	    {
#ifdef DOJDEBUG
		cerr << "memorymap(): could not fstat " << filename << " : " << strerror(errno) << endl;
#endif
		return 0;
	    }
	if(!S_ISREG(buf.st_mode))
	    {
#ifdef DOJDEBUG
		cerr << "memorymap(): " << filename << " is not a regular file" << endl;
#endif
		return 0;
	    }
	info.filelen=static_cast<uint64_t>(buf.st_size);

	// open file
	info.fh=::open(filename, info.readonly?O_RDONLY:O_RDWR);
	if(info.fh<0)
	    {
#ifdef DOJDEBUG
		cerr << "memorymap(): could not open " << filename << " : " << strerror(errno) <<  endl;
#endif
		return 0;
	    }

	// memory map file
	//lint -esym(953,w) w should be non const
	void *w=mmap(NULL, static_cast<unsigned>(info.filelen), info.readonly?PROT_READ:(PROT_READ|PROT_WRITE), info.readonly?MAP_PRIVATE:MAP_SHARED, info.fh, static_cast<off_t>(0));
	if(!w)
	    {
		const int e=errno;
#ifdef DOJDEBUG
		cerr << "memorymap(): could not mmap " << filename << " : " << strerror(errno) <<  endl;
#endif
		::close(info.fh);
		errno=e;
		return 0;
	    }

	// make an entry into registry
	memorymap_registry[w]=info;

	return w;
    }

    int memoryunmap(void *mem)
    {
	if(memorymap_registry.count(mem))
	    {
		struct mmap_info info=memorymap_registry[mem];
		//lint -e{534} ignore return value
		memorymap_registry.erase(mem);
		if(!info.readonly)
		    {
			if(msync(mem, static_cast<unsigned>(info.filelen), 0) < 0)
			    {
#ifdef DOJDEBUG
				clog << "memoryunmap(): could not msync()" << endl;
#endif
			    }
		    }
		if(munmap(mem, static_cast<unsigned>(info.filelen)) < 0)
		    {
#ifdef DOJDEBUG
			cerr << "memoryunmap(): could not unmap " << info.filename << " : " << strerror(errno) << endl;
#endif
		    }
		if(::close(info.fh) < 0)
		    {
#ifdef DOJDEBUG
			cerr << "memoryunmap(): could not close " << info.filename << " : " << strerror(errno) << endl;
#endif
		    }
		return 0;
	    }

#ifdef DOJDEBUG
	cerr << "memoryunmap(): pointer not found in registry" << endl;
#endif
	return -1;
    }
#endif // __unix__

#ifdef _WIN32
    void* memorymap(const char* filename, const bool readonly)
    {
	struct mmap_info info;
	info.filename=filename;

	info.file=CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN, 0);
	if(info.file==INVALID_HANDLE_VALUE)
	    {
#ifdef DOJDEBUG
		cerr << " could not open " << GetLastError() << endl;
#endif
		return 0;
	    }

	info.map=CreateFileMapping(info.file, NULL, PAGE_READONLY, 0, 0, filename);
	if(info.map==INVALID_HANDLE_VALUE)
	    {
#ifdef DOJDEBUG
		cerr << " could not create map " << GetLastError() << endl;
#endif
		CloseHandle(info.file);
		return 0;
	    }

	void *mem=MapViewOfFile(info.map, FILE_MAP_READ, 0, 0, 0);
	if(!mem)
	    {
#ifdef DOJDEBUG
		cerr << " could not map file " << GetLastError() << endl;
#endif
		CloseHandle(info.map);
		CloseHandle(info.file);
		return 0;
	    }

	memorymap_registry[mem]=info;
	return mem;
    }

    int memoryunmap(void *mem)
    {
	if(memorymap_registry.count(mem))
	    {
		struct mmap_info info=memorymap_registry[mem];
		memorymap_registry.erase(mem);
		UnmapViewOfFile(mem);
		CloseHandle(info.map);
		CloseHandle(info.file);
		return 0;
	    }

#ifdef DOJDEBUG
	cerr << "memoryunmap(): pointer not found in registry" << endl;
#endif
	return -1;
    }
#endif // _WIN32

    uint64_t memorymapsize(void *mem)
    {
	return (memorymap_registry.count(mem)) ? memorymap_registry[mem].filelen : 0;
    }

}
