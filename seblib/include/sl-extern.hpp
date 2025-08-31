#ifndef SEBLIB_EXTERN_HPP_
#define SEBLIB_EXTERN_HPP_

// required to prevent header clashes between raylib.h and windows.h
#if defined(_WIN32) || defined(__CYGWIN__)
#define SLHR_EXPORT extern "C" __declspec(dllexport)
#else
#define SLHR_EXPORT extern "C" __attribute__((visibility("default")))
#endif

#endif
