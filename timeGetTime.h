/* -*- mode: C++; c-basic-offset: 4; tab-width: 8; -*-
 * vi: set shiftwidth=4 tabstop=8:
 * :indentSize=4:tabSize=8:

Copyright (c) 2004..2009 Dirk Jagdmann <doj@cubic.org>

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

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#if defined(__glut_h__)
/// @return the number of milliseconds since program start.
inline uint32_t timeGetTime()
{
  return glutGet(static_cast<GLenum>(GLUT_ELAPSED_TIME));
}
#elif defined(__FreeBSD__) || defined(__linux__)
#include <sys/time.h>
inline uint32_t timeGetTime()
{
  struct timeval tv;
  if(gettimeofday(&tv, NULL) == 0)
    return static_cast<unsigned>(tv.tv_sec*1000 + tv.tv_usec/1000);
  return 0;
}
/**
   @return a 64bit unsigned number which upper 32bit increment every
   second and represent unix epoch time. The lower 32bit contain an
   approximate of the fraction of a second.
*/
inline uint64_t timeGetTime64()
{
  //lint --e{571} casts are ok
  struct timeval tv;
  if(gettimeofday(&tv, NULL) == 0)
    return (static_cast<uint64_t>(tv.tv_sec)<<32) | (static_cast<uint64_t>(tv.tv_usec)<<8);
  return 0;
}
#elif defined(CLOCK_MONOTONIC) && defined(_POSIX_TIMERS) && _POSIX_TIMERS>0
inline uint32_t timeGetTime()
{
  struct timespec tv;
  if(clock_gettime(CLOCK_MONOTONIC, &tv) == 0)
    return static_cast<unsigned>(tv.tv_sec*1000 + tv.tv_nsec/1000000);
  return 0;
}
inline uint64_t timeGetTime64()
{
  //lint --e{571} casts are ok
  struct timespec tv;
  if(clock_gettime(CLOCK_MONOTONIC, &tv) == 0)
    return (static_cast<uint64_t>(tv.tv_sec)<<32) | static_cast<uint64_t>(tv.tv_nsec);
  return 0;
}
#elif defined(__unix__)
#include <sys/timeb.h>
/// @return a number which is incremented every millisecond.
inline uint32_t timeGetTime()
{
  struct timeb t;
  ftime(&t);
  return (t.time&0xfffff)*1000 + t.millitm;
}
// TODO: timeGetTime64()
#elif defined(_WIN32)
#include <windows.h>
#include <mmsystem.h>

inline uint64_t timeGetTime64()
{
  SYSTEMTIME t;
  GetSystemTime(&t);
  return (static_cast<uint64_t>(time(NULL))<<32)|(t.wMilliseconds << 22);
}

#else
#error unknown platform. Can not provide timeGetTime()
#endif
