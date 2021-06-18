#pragma once

#include "common_defines.h"
#include "glsl_type.h"

struct Texture {
	Handle id;
};

enum MaterialField : int {
	MF_DIFFUSE0 = 0,
	MF_DIFFUSE1,
	MF_DIFFUSE2,
	MF_SPECULAR0,
	MF_SPECULAR1,
	MF_EMISSION,

	MF_TEXTURES_CNT, // Count of textures

	MF_SHININESS = MF_TEXTURES_CNT, //Other fields begin here

	MF_CNT
};

static FieldType materialField2Type[MF_CNT] = {
	fieldType_INT,
	fieldType_INT,
	fieldType_INT,
	fieldType_INT,
	fieldType_INT,
	fieldType_INT,
	fieldType_FLOAT
};

static const char *materialFieldMap[MF_CNT] = {
	"diffuse0",
	"diffuse1",
	"diffuse2",
	"specular0",
	"specular1",
	"emission",
	"shininess"
};

struct Material : GLSLVariable {
	Texture diffuse[3];
	Texture specular[2];
	Texture emission;
	float shininess;

	Material() : GLSLVariable("") { }
	Material(const String &name, float shininess = 32.f) : GLSLVariable(name), shininess(shininess) { }

	String getFieldName(FieldName fieldName) const override {
		char n[128];
		switch (fieldName) {
		case MF_DIFFUSE0:
		case MF_DIFFUSE1:
		case MF_DIFFUSE2:
		case MF_SPECULAR0:
		case MF_SPECULAR1:
		case MF_EMISSION:
		case MF_SHININESS:
			sprintf(n, "%s.%s", name.c_str(), materialFieldMap[fieldName]);
			return String(n);
		default:
			return String();
		}
	}

	Maybe<int> getIntField(FieldName fieldName) const override {
		switch (fieldName) {
		case MF_DIFFUSE0:
		case MF_DIFFUSE1:
		case MF_DIFFUSE2:
			return diffuse[fieldName].id;
		case MF_SPECULAR0:
		case MF_SPECULAR1:
			return specular[fieldName-MF_SPECULAR0].id;
		case MF_EMISSION:
			return emission.id;
		default:
			return Maybe<int>::getInvalid();
		}
	}

	Maybe<float> getFloatField(FieldName fieldName) const override {
		return fieldName == MF_SHININESS ? Maybe<float>(shininess) : Maybe<float>::getInvalid();
	}

	void setFields(Shader &shader) const override { }
};