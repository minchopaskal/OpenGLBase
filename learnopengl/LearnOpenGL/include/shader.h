// Credits: learnopengl.com

#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "utility.h"

#include "glsl_type.h"

struct Shader {
private:
	mutable Map<size_t, Handle> locationsCache;

public:
	unsigned int ID;
	Shader() : ID(-1) { }
	Shader(const char* vertexPath, const char *geometryPath, const char* fragmentPath) {
		init(vertexPath, geometryPath, fragmentPath);
	}

	void init(const char* vertexPath, const char *geometryPath, const char* fragmentPath) {
		std::string vertexCode;
		std::string fragmentCode;
		std::string geometryCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		std::ifstream gShaderFile;
	
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try {
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			if (geometryPath != nullptr) {
				gShaderFile.open(geometryPath);
			}
			std::stringstream vShaderStream, fShaderStream, gShaderStream;
	
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			if (geometryPath != nullptr) {
				gShaderStream << gShaderFile.rdbuf();
			}

			vShaderFile.close();
			fShaderFile.close();
			if (geometryPath != nullptr) {
				gShaderFile.close();
			}
			
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
			if (geometryPath != nullptr) {
				geometryCode = gShaderStream.str();
			}
		}
		catch (std::ifstream::failure e) {
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ::" << fragmentPath << std::endl;
		}
		const char *vShaderCode = vertexCode.c_str();
		const char *fShaderCode = fragmentCode.c_str();
		const char *gShaderCode = geometryPath != nullptr ? geometryCode.c_str() : nullptr;
		
		Handle vertex, fragment, geometry;
		// vertex shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		checkCompileErrors(vertex, "VERTEX");

		// geometry shader
		if (geometryPath != nullptr) {
			geometry = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geometry, 1, &gShaderCode, NULL);
			glCompileShader(geometry);
			checkCompileErrors(geometry, "GEOMETRY");
		}

		// fragment Shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		checkCompileErrors(fragment, "FRAGMENT");

		// shader Program
		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		if (geometryPath != nullptr) {
			glAttachShader(ID, geometry);
		}
		glAttachShader(ID, fragment);
		glLinkProgram(ID);
		checkCompileErrors(ID, "PROGRAM");

		// delete the shaders as they're linked into our program now and no longer necessery
		glDeleteShader(vertex);
		glDeleteShader(fragment);
		if (geometryPath != nullptr) {
			glDeleteShader(geometry);
		}
	}
	// activate the shader
	// ------------------------------------------------------------------------
	void use() const {
		glUseProgram(ID);
	}
	// utility uniform functions
	// ------------------------------------------------------------------------
	void setBool(const std::string &name, bool value) const {
		glUniform1i(getUniformLocation(name), (int)value);
	}
	// ------------------------------------------------------------------------
	void setInt(const std::string &name, int value) const {
		int i = getUniformLocation(name);
		glUniform1i(i, value);
	}
	// ------------------------------------------------------------------------
	void setFloat(const std::string &name, float value) const {
		glUniform1f(getUniformLocation(name), value);
	}
	// ------------------------------------------------------------------------
	void setVec2(const std::string &name, const glm::vec2 &value) const {
		glUniform2fv(getUniformLocation(name), 1, &value[0]);
	}
	void setVec2(const std::string &name, float x, float y) const {
		glUniform2f(getUniformLocation(name), x, y);
	}
	// ------------------------------------------------------------------------
	void setVec3(const std::string &name, const glm::vec3 &value) const {
		glUniform3fv(getUniformLocation(name), 1, &value[0]);
	}
	void setVec3(const std::string &name, float x, float y, float z) const {
		glUniform3f(getUniformLocation(name), x, y, z);
	}
	// ------------------------------------------------------------------------
	void setVec4(const std::string &name, const glm::vec4 &value) const {
		glUniform4fv(getUniformLocation(name), 1, &value[0]);
	}
	void setVec4(const std::string &name, float x, float y, float z, float w) const {
		glUniform4f(getUniformLocation(name), x, y, z, w);
	}
	// ------------------------------------------------------------------------
	void setMat2(const std::string &name, const glm::mat2 &mat) const {
		glUniformMatrix2fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void setMat3(const std::string &name, const glm::mat3 &mat) const {
		glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void setMat4(const std::string &name, const glm::mat4 &mat) const {
		int i = getUniformLocation(name);
		glUniformMatrix4fv(i, 1, GL_FALSE, &mat[0][0]);
	}

	void setField(const GLSLVariable &v, FieldName fieldName, FieldType fieldType) {
		String name = v.getFieldName(fieldName);
		if (name.empty()) {
			return;
		}

		switch (fieldType) {
		case fieldType_BOOL:
			setBool(name, v.getBoolField(fieldName).get());
			break;
		case fieldType_INT:
			setInt(name, v.getIntField(fieldName).get());
			break;
		case fieldType_FLOAT:
			setFloat(name, v.getFloatField(fieldName).get());
			break;
		case fieldType_VEC2:
			setVec2(name, v.getVec2Field(fieldName).get());
			break;
		case fieldType_VEC3:
			setVec3(name, v.getVec3Field(fieldName).get());
			break;
		case fieldType_VEC4:
			setVec4(name, v.getVec4Field(fieldName).get());
			break;
		case fieldType_MAT2:
			setMat2(name, v.getMat2Field(fieldName).get());
			break;
		case fieldType_MAT3:
			setMat3(name, v.getMat3Field(fieldName).get());
			break;
		case fieldType_MAT4:
			setMat4(name, v.getMat4Field(fieldName).get());
			break;
		default:
			break;
		}
	}

private:
	// utility function for checking shader compilation/linking errors.
	// ------------------------------------------------------------------------
	void checkCompileErrors(Handle shader, std::string type) {
		GLint success;
		GLchar infoLog[1024];
		if (type != "PROGRAM") {
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success) {
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
		else {
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR in " << ID << " of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
	}

	unsigned int getUniformLocation(const String &name) const {
		size_t hash = getStringHash(name);
		
		if (locationsCache.find(hash) != locationsCache.end()) {
			return locationsCache.at(hash);
		}

		unsigned int res = glGetUniformLocation(ID, name.c_str());
		locationsCache[hash] = res;
		return res;
	}
};

#endif