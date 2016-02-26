//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

#ifndef _MURMURHASH3_H_
#define _MURMURHASH3_H_

//-----------------------------------------------------------------------------
// Platform-specific functions and macros

// Microsoft Visual Studio

#if defined(_MSC_VER) && (_MSC_VER < 1600)

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;

// Other compilers

#else	// defined(_MSC_VER)

#include <stdint.h>

#endif // !defined(_MSC_VER)

#include <string>

//return type for x86 128b functions
struct Int128_x86_t
{
    uint32_t data[4];
};

//return type for x64 128b functions
struct Int128_x64_t
{
    uint64_t data[2];
};


//-----------------------------------------------------------------------------

uint32_t murmurHash3_x86_32 (std::string key, uint32_t seed = 0);
Int128_x86_t murmurHash3_x86_128 (std::string key, uint32_t seed = 0);
Int128_x64_t murmurHash3_x64_128 (std::string ke, uint32_t seed = 0);

//-----------------------------------------------------------------------------

#endif // _MURMURHASH3_H_
