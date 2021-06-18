#ifndef GLSL_TYPE_H
#define GLSL_TYPE_H

#include "common_defines.h"

struct Shader;

enum FieldType {
	fieldType_BOOL = 0,
	fieldType_INT,
	fieldType_FLOAT,
	fieldType_VEC2,
	fieldType_VEC3,
	fieldType_VEC4,
	fieldType_MAT2,
	fieldType_MAT3,
	fieldType_MAT4,

	fieldType_CNT // count of field types
};

using FieldName = int;

template <class T>
struct Maybe {
private:
	constexpr static int DATA_SIZE = sizeof(T) + (sizeof(size_t) - sizeof(T) % sizeof(size_t));
	char data[DATA_SIZE];
	bool valid;

	void copyData(const T &data) {
		memcpy(this->data, &data, sizeof(T));
	}

public:
	Maybe() : valid(false) { }

	Maybe(const T &data, bool valid = true) : valid(valid) {
		if (valid) {
			copyData(data);
		}
	}

	static Maybe<T> getValid(const T &data) {
		return { data, true };
	}

	static Maybe<T> getInvalid() {
		return { };
	}

	operator bool() const {
		return isValid();
	}

	bool isValid() const {
		return valid;
	}

	const T& get() const {
		assert(valid && "Maybe value not valid!");
		return *reinterpret_cast<const T*>(data);
	}
};

struct GLSLVariable {
	String name; // Name of the uniform in glsl shader

	GLSLVariable() = default;
	GLSLVariable(const String &name) : name(name) { }

	bool valid() const {
		return !name.empty();
	}

	virtual String getFieldName(FieldName) const = 0 { return String(); }

	virtual Maybe<bool> getBoolField(FieldName fieldName) const { return Maybe<bool>::getInvalid(); }
	virtual Maybe<int> getIntField(FieldName fieldName) const { return Maybe<int>::getInvalid(); }
	virtual Maybe<float> getFloatField(FieldName fieldName) const { return Maybe<float>::getInvalid(); }
	virtual Maybe<Vec2> getVec2Field(FieldName fieldName) const { return Maybe<Vec2>::getInvalid(); }
	virtual Maybe<Vec3> getVec3Field(FieldName fieldName) const { return Maybe<Vec3>::getInvalid(); }
	virtual Maybe<Vec4> getVec4Field(FieldName fieldName) const { return Maybe<Vec4>::getInvalid(); }
	virtual Maybe<Mat2> getMat2Field(FieldName fieldName) const { return Maybe<Mat2>::getInvalid(); }
	virtual Maybe<Mat3> getMat3Field(FieldName fieldName) const { return Maybe<Mat3>::getInvalid(); }
	virtual Maybe<Mat4> getMat4Field(FieldName fieldName) const { return Maybe<Mat4>::getInvalid(); }

	virtual void setFields(Shader &shader) const = 0;
};

#endif // GLSL_TYPE_H