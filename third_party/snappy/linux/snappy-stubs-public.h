// Copyright 2011 Google Inc. All Rights Reserved.
// Author: sesse@google.com (Steinar H. Gunderson)
//
// Various type stubs for the open-source version of Snappy.
//
// This file cannot include config.h, as it is included from snappy.h,
// which is a public header. Instead, snappy-stubs-public.h is generated by
// from snappy-stubs-public.h.in at configure time.

#ifndef UTIL_SNAPPY_OPENSOURCE_SNAPPY_STUBS_PUBLIC_H_
#define UTIL_SNAPPY_OPENSOURCE_SNAPPY_STUBS_PUBLIC_H_

#if 1
#include <stdint.h>
#endif

#if 1
#include <stddef.h>
#endif

#define SNAPPY_MAJOR 1
#define SNAPPY_MINOR 0
#define SNAPPY_PATCHLEVEL 0
#define SNAPPY_VERSION \
    ((SNAPPY_MAJOR << 16) | (SNAPPY_MINOR << 8) | SNAPPY_PATCHLEVEL)

#include <string>

namespace snappy {

#if 1
typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;
#else
typedef signed char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
typedef long long int64;
typedef unsigned long long uint64;
#endif

typedef std::string string;

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

}  // namespace snappy

#endif  // UTIL_SNAPPY_OPENSOURCE_SNAPPY_STUBS_PUBLIC_H_
