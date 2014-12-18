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

#pragma once

#include <stdint.h>
#include <string>

namespace doj
{

    /**
       map filename into memory. The size of the memory area can be
       retrieved with the memorymapsize(void*) function.
       @param filename pathname of file on local filesystem
       @param readonly if true map filename exclusive and write protected, if false map filename shared and writable
       @return a pointer to the memory mapped file, NULL on error
    */
    void* memorymap(const char *filename, const bool readonly=true);

    /**
       map filename into memory. The size of the memory area can be
       retrieved with the memorymapsize(void*) function.
       @param filename pathname of file on local filesystem
       @param readonly if true map filename exclusive and write protected, if false map filename shared and writable
       @return a pointer to the memory mapped file, NULL on error
    */
    inline void* memorymap(const std::string& filename, const bool readonly=true)
    {
	return memorymap(filename.c_str(), readonly);
    }

    /**
       unmap a previously memory mapped area.

       @param mem pointer to the beginning of the memory mapped file, as
       retrieved from the memorymap(...) function.

       @return 0 on success. -1 on failure.
    */
    int memoryunmap(void *mem);

    /**
       get the size of a memory mapped file.

       @param mem pointer to the beginning of the memory mapped file, as
       retrieved from the memorymap(...) function.

       @return the size of the memory area, which is identical to the
       filesize. 0 if the *mem does not resolve a valid start of a memory
       map.
    */
    uint64_t memorymapsize(void *mem);


    /** a wrapper class for doj::memorymap which acts like a pointer. If
	the object goes out of scope the underlying memory is
	unmapped.
    */
    template <typename T>
    class memorymap_ptr
    {
	//lint --e{1740} we don't need to free anything about *p

	T *p;			///< pointer to beginning of mapped file

    public:
	typedef T element_type;	///< makes template parameter T accessible

	typedef T* iterator;
	typedef const T* const_iterator;

	/** construct a memory map from a file.
	    @param fn filename of file in local file system.
	    @param readonly if true the file is mapped with write protection and exclusive access, if false the memory area can be written to and the file is mapped shared.
	*/
	explicit memorymap_ptr(const std::string& fn, const bool readonly=true) :
	    p(reinterpret_cast<T*>(memorymap(fn.c_str(), readonly)))
	{ }

	~memorymap_ptr() {
	    try {
		//lint -e{534} ignore return value
		memoryunmap(p);
	    } catch(...) {
		// \todo this should not happen
	    }
	}

	/// @return size of mapped file in bytes
	uint64_t size() const { return memorymapsize(p); }

	/// @return mapped file as object reference
	T& operator*() { return *p; }
	/// @return pointer to beginning of mapped file
	T* operator->() const { return p; }
	/// @return pointer to beginning of mapped file
	T* get() const { return p; }

	/// @return true if no file is mapped
	bool operator!() const { return !p; }

	/// @return true if no file is mapped, or file has length of 0 bytes
	bool empty() const { return !p || size()==0; }

	iterator begin() const { return p; }
	iterator end() const { return p + size(); }

    private:
	//lint --e{1704} we want those function to be private
	memorymap_ptr();
	memorymap_ptr(const memorymap_ptr&);
	memorymap_ptr& operator=(const memorymap_ptr&);
    };

}
