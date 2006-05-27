#ifndef _GLIBMM_CONFIG_H
#define _GLIBMM_CONFIG_H 1

#include "config.hh"

/* version numbers */
#undef GLIBMM_MAJOR_VERSION
#undef GLIBMM_MINOR_VERSION
#undef GLIBMM_MICRO_VERSION

// detect common platforms
#if defined(_WIN32)
// Win32 compilers have a lot of varation
#if defined(_MSC_VER)
#define GLIBMM_MSC
#define GLIBMM_WIN32
#define GLIBMM_DLL
#elif defined(__CYGWIN__)
#define GLIBMM_GCC
#elif defined(__MINGW32__)
#define GLIBMM_WIN32
#define GLIBMM_GCC
#define GLIBMM_DLL
#else
#warning "Unknown architecture (send me gcc --dumpspecs or equiv)"
#endif
#else
#define GLIBMM_GCC
#endif /* _WIN32 */

#ifdef GLIBMM_MSC
#define GLIBMM_CXX_HAVE_MUTABLE
#define GLIBMM_CXX_HAVE_NAMESPACES
#pragma warning (disable: 4786 4355 4800 4181)
#endif

#ifndef GLIBMM_HAVE_NAMESPACE_STD
#  define GLIBMM_USING_STD(Symbol) namespace std { using ::Symbol; }
#else
#  define GLIBMM_USING_STD(Symbol) /* empty */
#endif

#ifdef GLIBMM_DLL
#if defined(glibmm_COMPILATION) && defined(DLL_EXPORT)
#define GLIBMM_API __declspec(dllexport) 
#elif !defined(glibmm_COMPILATION)
#define GLIBMM_API __declspec(dllimport)
#else
#define GLIBMM_API
#endif /* glibmm_COMPILATION - DLL_EXPORT */
#else
#define GLIBMM_API
#endif /* GLIBMM_DLL */

#endif /* _GLIBMM_CONFIG_H */

