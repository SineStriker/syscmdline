#ifndef GLOBAL_H
#define GLOBAL_H

// Export define
#ifdef _WIN32
#  define SYSCMDLINE_DECL_EXPORT __declspec(dllexport)
#  define SYSCMDLINE_DECL_IMPORT __declspec(dllimport)
#else
#  define SYSCMDLINE_DECL_EXPORT
#  define SYSCMDLINE_DECL_IMPORT
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

#endif // GLOBAL_H
