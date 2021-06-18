#pragma once

#include "common_defines.h"

template <class T>
T Max(const T &x, const T &y) {
	return x > y ? x : y;
}

template <class T>
T Min(const T &x, const T &y) {
	return x < y ? x : y;
}

// cp-algorithms
size_t getStringHash(const String &str);