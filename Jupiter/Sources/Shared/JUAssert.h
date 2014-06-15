
#ifndef __JUASSERT_H__
#define __JUASSERT_H__

#include <assert.h>

#define JUAssert(x, desc)  (void)( (!!(x)) || (_wassert(_CRT_WIDE(#x##L" -> "##L##desc), _CRT_WIDE(__FILE__), __LINE__), 0) )

#endif