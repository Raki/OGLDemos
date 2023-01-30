#pragma once
#include <GL/glew.h>
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <glm/glm.hpp>

using std::string;
using std::vector;
using std::map;

class GlslProgram
{
	map<string, glm::vec2> vec2Map;
	map<string, glm::vec3> vec3Map;
	map<string, glm::vec4> vec4Map;
	map<string, glm::mat4> mat4Map;
	map<string, float> floatMap;
	map<string, int> intMap;
	map<string, GLint> uniformLocations;
	map<string, GLuint> subroutineLocations;
	
	GLuint vShaderID;
	GLuint fShaderID;
	GLuint gShaderID;
	GLuint cShaderID;
public:
	GLuint programID;
	GlslProgram(string vShaderStr, string fShaderStr);
	GlslProgram(string vShaderStr, string gShaderStr,string fShaderStr);
	GlslProgram(string cShaderStr);

	GLuint createShader(const string shaderStr, GLenum type);
	void freeShaders();
	void bind();
	void unbind();

	void setVec2f(string name, glm::vec2 val);
	void bindVec2f(string name, glm::vec2 val);
	void setVec3f(string name, glm::vec3 val);
    void bindVec3f(string name, glm::vec3 val);
	void setVec4f(string name, glm::vec4 val);
	void bindVec4f(string name, glm::vec4 val);
    void setMat4f(string name, glm::mat4 val);
    void bindMat4f(string name, glm::mat4 val);
    void setFloat(string name, float val);
    void bindFloat(string name, float val);
    void setInt(string name, int val);
    void bindInt(string name, int val);
	void activeSubrountine(string name, GLenum shaderType);
	void bindAllUniforms();

	~GlslProgram();
};

