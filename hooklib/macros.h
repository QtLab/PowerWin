///
/// Copyright (c) 2016 R1tschY
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///

#pragma once

#include <cpp-utils/preprocessor.h>

// /////////////////////////////////////////////////////////////////////////////
// Check for 64bit

// Check GCC
#ifdef __GNUC__
  #if defined(__x86_64__) || defined(__ppc64__)
    #define CPUBITSET 64
  #else
    #define CPUBITSET 32
  #endif
#else

  // Check windows
  #ifdef _WIN32
    #ifdef _WIN64
      #define CPUBITSET 64
    #else
      #define CPUBITSET 32
  #endif
#endif

#endif

#ifndef CPUBITSET
    #error "Must define CPUBITSET"
#endif


#if CPUBITSET == 32
# define POWERWIN_APP_NAME "PowerWin32"
# define POWERWIN_64BIT_NAME "PowerWin64"
#elif CPUBITSET == 64
# define POWERWIN_APP_NAME "PowerWin64"
#endif

#ifdef BUILD_DLL
#define DLL_PUBLIC __declspec(dllexport)
#else
#define DLL_PUBLIC __declspec(dllimport)
#endif
