#include "utility.h"

// cp-algorithms
size_t getStringHash(const String &str) {
	const int p = 31;
	const int m = int(1e9) + 9;
	size_t hash_value = 0;
	size_t p_pow = 1;
	for (char c : str) {
		hash_value = (hash_value + (c - 'a' + 1) * p_pow) % m;
		p_pow = (p_pow * p) % m;
	}
	return hash_value;
}