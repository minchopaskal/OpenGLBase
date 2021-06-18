#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

template <class T>
using Vec = std::vector<T>;

template <class T>
using Set = std::unordered_set<T>;

template <class K, class V>
using Map = std::unordered_map<K, V>;

using String = std::string;

using Vec2 = glm::vec<2, float>;
using Vec3 = glm::vec<3, float>;
using Vec4 = glm::vec<4, float>;

using Mat2 = glm::mat2;
using Mat3 = glm::mat3;
using Mat4 = glm::mat4;

using Handle = unsigned int;
