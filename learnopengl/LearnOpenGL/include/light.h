#ifndef LIGHT_H
#define LIGHT_H

#include "common_defines.h"
#include "drawable.h"
#include "model.h"
#include "glsl_type.h"

enum LightField : FieldName {
	LF_AMBIENT = 0,
	LF_DIFFUSE,
	LF_SPECULAR,
	LF_DIRECTION,
	LF_POSITION,
	LF_ATTENUATION,
	LF_CUTOFF,

	LF_CNT
};

static const char *lightFieldMap[LF_CNT] = {
	"ambient",
	"diffuse",
	"specular",
	"direction",
	"position",
	"attenuation",
	"cutoff"
};

const Vec3 InfVec = Vec3(INFINITY);

struct Light : GLSLVariable, DrawableInterface {
	Vec3 ambient;
	Vec3 diffuse;
	Vec3 specular;

	Light(const String &name, Vec3 a, Vec3 d, Vec3 s) : GLSLVariable(name), ambient(a), diffuse(d), specular(s) {}

	virtual ~Light() {}

	virtual String getFieldName(FieldName fieldName) const override {
		char n[128];
		
		switch (fieldName) {
		case LF_AMBIENT:
		case LF_DIFFUSE:
		case LF_SPECULAR:
			sprintf(n, "%s.light.%s", name.c_str(), lightFieldMap[fieldName]);
			break;
		default:
			return String();
		}

		return String(n);
	}

	virtual Maybe<Vec3> getVec3Field(FieldName fieldName) const override {
		switch (fieldName) {
		case LF_AMBIENT:
			return ambient;
		case LF_DIFFUSE:
			return diffuse;
		case LF_SPECULAR:
			return specular;
		default:
			return Maybe<Vec3>::getInvalid();
		}
	}

	virtual void setFields(Shader &shader) const override {
		shader.setVec3(getFieldName(LF_AMBIENT), getVec3Field(LF_AMBIENT).get());
		shader.setVec3(getFieldName(LF_DIFFUSE), getVec3Field(LF_AMBIENT).get());
		shader.setVec3(getFieldName(LF_SPECULAR), getVec3Field(LF_AMBIENT).get());
	}
};

struct DirectionalLight : Light {
	Vec3 direction;

	DirectionalLight(const String &name, Vec3 a, Vec3 d, Vec3 s, Vec3 dir) : Light(name, a, d, s), direction(dir) {}
	virtual ~DirectionalLight() {}

	String getFieldName(FieldName fieldName) const override {
		String result = Light::getFieldName(fieldName);
		if (!result.empty()) {
			return result;
		}

		char n[128];
		if (fieldName == LF_DIRECTION) {
			sprintf(n, "%s.direction", name.c_str());
			return String(n);
		}

		return String();
	}

	virtual Maybe<Vec3> getVec3Field(FieldName fieldName) const override {
		auto res = Light::getVec3Field(fieldName);
		if (res.isValid()) {
			return res;
		}

		return fieldName == LF_DIRECTION ? direction : Maybe<Vec3>::getInvalid();
	}

	virtual void draw(Shader &shader) const override { }

	virtual void setFields(Shader &shader) const override {
		Light::setFields(shader);
		shader.setVec3(getFieldName(LF_DIRECTION), getVec3Field(LF_DIRECTION).get());
	}
};

struct PointLight : Light {
	Model *model;
	Vec3 position;
	Vec3 attenuation; // constant, linear and quadratic constants for the attenuation calculation

	PointLight(const String &name, Model *model, Vec3 a, Vec3 d, Vec3 s, Vec3 pos, Vec3 att) :
		Light(name, a, d, s),
		model(model),
		position(pos),
		attenuation(att) { }
	virtual ~PointLight() {}

	virtual String getFieldName(FieldName fieldName) const override {
		String result = Light::getFieldName(fieldName);
		if (!result.empty()) {
			return result;
		}

		char n[128];
		switch (fieldName) {
		case LF_POSITION:
		case LF_ATTENUATION:
			sprintf(n, "%s.%s", name.c_str(), lightFieldMap[fieldName]);
			break;
		default:
			return String();
		}

		return String(n);
	}

	virtual Maybe<Vec3> getVec3Field(FieldName fieldName) const override {
		Maybe<Vec3> res = Light::getVec3Field(fieldName);
		if (res.isValid()) {
			return res;
		}

		switch (fieldName) {
		case LF_POSITION: return position;
		case LF_ATTENUATION: return attenuation;
		default: return Maybe<Vec3>::getInvalid();
		}
	}

	virtual void draw(Shader &shader) const override {
		if (model) {
			model->draw(shader);
		}
	}

	virtual void setFields(Shader &shader) const override {
		Light::setFields(shader);
		shader.setVec3(getFieldName(LF_POSITION), getVec3Field(LF_POSITION).get());
		shader.setVec3(getFieldName(LF_ATTENUATION), getVec3Field(LF_ATTENUATION).get());
	}
};

struct SpotLight : PointLight {
private:
	Vec3 direction;
	float cutoff; // cosine of the cutoff angle
	const Camera &camera;

public:
	SpotLight(const String &name, const Camera &cam, Vec3 a, Vec3 d, Vec3 s, Vec3 att, float cutoff) :
		PointLight(name, nullptr, a, d, s, Vec3(0.f), att),
		camera(cam),
		direction(Vec3(0.f)),
		cutoff(cutoff) {}

	virtual String getFieldName(FieldName fieldName) const override {
		String result = PointLight::getFieldName(fieldName);
		if (!result.empty()) {
			return result;
		}

		char n[128];
		switch (fieldName) {
		case LF_DIRECTION:
		case LF_CUTOFF:
			sprintf(n, "%s.%s", name.c_str(), lightFieldMap[fieldName]);
			break;
		default:
			return String();
		}

		return String(n);
	}

	Maybe<Vec3> getVec3Field(FieldName fieldName) const override {
		Maybe<Vec3> res = Light::getVec3Field(fieldName);
		if (res.isValid()) {
			return res;
		}

		switch (fieldName) {
		case LF_POSITION: return camera.Position;
		case LF_ATTENUATION: return attenuation;
		case LF_DIRECTION: return camera.Front;
		default: return Maybe<Vec3>::getInvalid();
		}
	}

	Maybe<float> getFloatField(FieldName fieldName) const override {
		return (fieldName == LF_CUTOFF) ? Maybe<float>(cutoff) : Maybe<float>::getInvalid();
	}

	void draw(Shader &shader) const override { }

	virtual void setFields(Shader &shader) const override {
		Light::setFields(shader);
		shader.setVec3(getFieldName(LF_POSITION), getVec3Field(LF_POSITION).get());
		shader.setVec3(getFieldName(LF_ATTENUATION), getVec3Field(LF_ATTENUATION).get());
		shader.setVec3(getFieldName(LF_DIRECTION), getVec3Field(LF_DIRECTION).get());
		shader.setFloat(getFieldName(LF_CUTOFF), getFloatField(LF_CUTOFF).get());
	}
};

#endif // LIGHT_H