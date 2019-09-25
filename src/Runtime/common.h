#pragma once

#include <assert.h>


#define ARRAY_SIZE(Array) (sizeof(Array)/sizeof((Array)[0]))
#define MINI_ASSERT(condition, msgFmt, ...) \
assert(condition)