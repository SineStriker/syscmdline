#ifndef GLOBAL_H
#define GLOBAL_H

// Export define
#ifdef _MSC_VER
#  define SYSCMDLINE_DECL_EXPORT __declspec(dllexport)
#  define SYSCMDLINE_DECL_IMPORT __declspec(dllimport)
#else
#  define SYSCMDLINE_DECL_EXPORT __attribute__((visibility("default")))
#  define SYSCMDLINE_DECL_IMPORT __attribute__((visibility("default")))
#endif

#ifdef SYSCMDLINE_STATIC
#  define SYSCMDLINE_EXPORT
#else
#  ifdef SYSCMDLINE_LIBRARY
#    define SYSCMDLINE_EXPORT SYSCMDLINE_DECL_EXPORT
#  else
#    define SYSCMDLINE_EXPORT SYSCMDLINE_DECL_IMPORT
#  endif
#endif

// Utils
#define SYSCMDLINE_UNUSED(X) (void) X;

#endif // GLOBAL_H
