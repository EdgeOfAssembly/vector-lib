#ifndef __ALIGN_H__
#define __ALIGN_H__

/* Ensure that alignas, _Alignas, alignof, and _Alignof are not defined by previous includes */
#ifdef alignas
    #undef alignas
#endif
#ifdef _Alignas
    #undef _Alignas
#endif
#ifdef alignof
    #undef alignof
#endif
#ifdef _Alignof
    #undef _Alignof
#endif

/* Define alignas and _Alignas for all compilers and standards */
#if !defined(__cplusplus) || (__cplusplus < 201103L)
    /* For C or C++ pre-C++11 */
    #if defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER)
        /* GCC, Clang, and Intel Compiler use __attribute__ */
        #define alignas(x) __attribute__((aligned(x)))
        #define _Alignas alignas
        #define alignof(type) __alignof__(type)
        #define _Alignof alignof
    #elif defined(_MSC_VER)
        /* Microsoft Visual C++ uses __declspec */
        #define alignas(x) __declspec(align(x))
        #define _Alignas alignas
        #warning "alignof and _Alignof are not directly supported by MSVC before C++11. Alignment queries will not work."
        #define alignof(type)
        #define _Alignof alignof
    #else
        /* For other compilers where alignment isn't supported, define as no-op */
        #warning "This compiler does not support alignment features. alignas, _Alignas, alignof, and _Alignof are defined as no-ops."
        #define alignas(x)
        #define _Alignas
        #define alignof(type)
        #define _Alignof
    #endif
#else
    /* For C++11 and later */
    #if defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER)
        /* For GCC, Clang, and Intel Compiler, use C++11's alignas and alignof unless explicitly overridden */
        #define alignas(x) alignas(x)
        #define _Alignas alignas
        #define alignof(type) alignof(type)
        #define _Alignof alignof
    #elif defined(_MSC_VER)
        /* MSVC supports alignas and alignof in C++11 mode */
        #define alignas(x) alignas(x)
        #define _Alignas alignas
        #define alignof(type) alignof(type)
        #define _Alignof alignof
    #else
        /* If the compiler doesn't support C++11 alignas or alignof, define as no-op */
        #warning "This compiler does not support C++11 alignment features. alignas, _Alignas, alignof, and _Alignof are defined as no-ops."
        #define alignas(x)
        #define _Alignas
        #define alignof(type)
        #define _Alignof
    #endif
#endif

#endif /* __ALIGN_H__ */

