#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define __TODO__

template <class T>
T Max(const T& a, const T& b) {
	return (a > b ? a : b);
}

template <class T>
T Min(const T& a, const T& b) {
	return (a < b ? a : b);
}

template <class T>
using Vec = std::vector<T>;

template <class T>
using Set = std::unordered_set<T>;

template <class K, class V>
using HashMap = std::unordered_map<K, V>;

template <class K, class V, class Hasher>
using HashMapCustom = std::unordered_map<K, V, Hasher>;

using Vec2i = glm::vec<2, int>;
using Vec3i = glm::vec<3, int>;
using Vec4i = glm::vec<4, int>;

template <int N>
using VecNi = glm::vec<N, int>;

using Vec3 = glm::vec<3, float>;
using Vec4 = glm::vec<4, float>;

using String = std::string;