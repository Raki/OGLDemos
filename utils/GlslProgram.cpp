#include "GlslProgram.h"


GlslProgram::GlslProgram(string vShaderStr, string fShaderStr)
{

	vShaderID = createShader(vShaderStr, GL_VERTEX_SHADER);
	fShaderID = createShader(fShaderStr, GL_FRAGMENT_SHADER);


	if (vShaderID > 0 && fShaderID > 0)
	{
		programID = glCreateProgram();
		glAttachShader(programID, vShaderID);
		glAttachShader(programID, fShaderID);
		glLinkProgram(programID);

		GLint isLinked = 0;
		glGetProgramiv(programID, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLength);

			if (maxLength > 0)
			{
				// The maxLength includes the NULL character
				std::vector<GLchar> infoLog(maxLength);
				glGetProgramInfoLog(programID, maxLength, &maxLength, &infoLog[0]);

				freeShaders();
				// The program is useless now. So delete it.
				glDeleteProgram(programID);

				// Provide the infolog in whatever manner you deem best.
				// Exit with failure.
				std::cout << "program linking error" << infoLog.data() << '\n';
				programID = 0;
			}
			else
			{
				freeShaders();
				// The program is useless now. So delete it.
				glDeleteProgram(programID);
				std::cout << "program linking error : no log string" << '\n';
			}
		}
		else
		{
			// Always detach shaders after a successful link.
			freeShaders();
		}
	}

	
}

//for compute shader
GlslProgram::GlslProgram(string cShaderStr)
{
	vShaderID = fShaderID = 0;
	cShaderID = createShader(cShaderStr, GL_COMPUTE_SHADER);

	if (cShaderID > 0)
	{
		programID = glCreateProgram();
		glAttachShader(programID, cShaderID);
		glLinkProgram(programID);

		GLint isLinked = 0;
		glGetProgramiv(programID, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLength);

			if (maxLength > 0)
			{
				// The maxLength includes the NULL character
				std::vector<GLchar> infoLog(maxLength);
				glGetProgramInfoLog(programID, maxLength, &maxLength, &infoLog[0]);

				freeShaders();
				// The program is useless now. So delete it.
				glDeleteProgram(programID);

				// Provide the infolog in whatever manner you deem best.
				// Exit with failure.
				std::cout << "program linking error" << infoLog.data() << '\n';
				programID = 0;
			}
			else
			{
				freeShaders();
				// The program is useless now. So delete it.
				glDeleteProgram(programID);
				std::cout << "program linking error : no log string" << '\n';
			}
		}
		else
		{
			// Always detach shaders after a successful link.
			freeShaders();
		}
	}
}



GLuint GlslProgram::createShader(const string shaderStr, GLenum type)
{
	const char* src = shaderStr.c_str();
	auto shader = glCreateShader(type);
	glShaderSource(shader, 1, (const GLchar* const*)&src, NULL);
	glCompileShader(shader);

	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);

	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

		// Provide the infolog in whatever manor you deem best.
		// Exit with failure.
		glDeleteShader(shader); // Don't leak the shader.

		std::cout << "glsl error" << errorLog.data() << '\n';

		return 0;
	}

	return shader;
}

void GlslProgram::freeShaders()
{
	if (programID > 0)
	{
		if(vShaderID>0)
			glDetachShader(programID, this->vShaderID);
		if (fShaderID > 0)
			glDetachShader(programID, this->fShaderID);
		if (cShaderID > 0)
			glDetachShader(programID, this->cShaderID);
	}

	if (vShaderID > 0)
	{
		glDeleteShader(vShaderID);
		vShaderID = 0;
	}

	if (fShaderID > 0)
	{
		glDeleteShader(fShaderID);
		fShaderID = 0;
	}
	if (cShaderID > 0)
	{
		glDeleteShader(cShaderID);
		cShaderID = 0;
	}
}

void GlslProgram::bind()
{
	if (programID <= 0)
		std::cout << "can't bind program " << programID << '\n';
	else
		glUseProgram(programID);
}

void GlslProgram::unbind()
{
	glUseProgram(0);
}

void GlslProgram::setVec2f(string name, glm::vec2 val)
{
	if (uniformLocations.find(name) == uniformLocations.end())
	{
		auto loc = glGetUniformLocation(programID, name.c_str());
		uniformLocations[name] = loc;
	}
	vec2Map[name] = val;
}

void GlslProgram::bindVec2f(string name, glm::vec2 val)
{
	if (uniformLocations.find(name) == uniformLocations.end())
	{
		auto loc = glGetUniformLocation(programID, name.c_str());
		uniformLocations[name] = loc;
	}
	vec2Map[name] = val;
	glUniform2fv(uniformLocations[name], 1, &val[0]);
}

void GlslProgram::setVec3f(string name, glm::vec3 val)
{
	if (uniformLocations.find(name) == uniformLocations.end())
	{
		auto loc = glGetUniformLocation(programID, name.c_str());
		uniformLocations[name] = loc;
	}
	vec3Map[name] = val;
}

void GlslProgram::bindVec3f(string name, glm::vec3 val)
{
	if (uniformLocations.find(name) == uniformLocations.end())
	{
		auto loc = glGetUniformLocation(programID, name.c_str());
		uniformLocations[name] = loc;
	}
	vec3Map[name] = val;
	glUniform3fv(uniformLocations[name], 1, &val[0]);
}

void GlslProgram::setVec4f(string name, glm::vec4 val)
{
	if (uniformLocations.find(name) == uniformLocations.end())
	{
		auto loc = glGetUniformLocation(programID, name.c_str());
		uniformLocations[name] = loc;
	}
	vec4Map[name] = val;
}

void GlslProgram::bindVec4f(string name, glm::vec4 val)
{
	if (uniformLocations.find(name) == uniformLocations.end())
	{
		auto loc = glGetUniformLocation(programID, name.c_str());
		uniformLocations[name] = loc;
	}
	vec4Map[name] = val;
	glUniform4fv(uniformLocations[name], 1, &val[0]);
}

void GlslProgram::setMat4f(string name, glm::mat4 val)
{
	if (uniformLocations.find(name) == uniformLocations.end())
	{
		auto loc = glGetUniformLocation(programID, name.c_str());
		uniformLocations[name] = loc;
	}
	mat4Map[name] = val;
}

void GlslProgram::bindMat4f(string name, glm::mat4 val)
{
	if (uniformLocations.find(name) == uniformLocations.end())
	{
		auto loc = glGetUniformLocation(programID, name.c_str());
		uniformLocations[name] = loc;
	}
	mat4Map[name] = val;
	glUniformMatrix4fv(uniformLocations[name], 1, GL_FALSE, &val[0][0]);
}

void GlslProgram::setFloat(string name, float val)
{
	if (uniformLocations.find(name) == uniformLocations.end())
	{
		auto loc = glGetUniformLocation(programID, name.c_str());
		uniformLocations[name] = loc;
	}
	floatMap[name] = val;
}

void GlslProgram::bindFloat(string name, float val)
{
	if (uniformLocations.find(name) == uniformLocations.end())
	{
		auto loc = glGetUniformLocation(programID, name.c_str());
		uniformLocations[name] = loc;
	}
	floatMap[name] = val;
	glUniform1f(uniformLocations[name], val);
}

void GlslProgram::setInt(string name, int val)
{
	if (uniformLocations.find(name) == uniformLocations.end())
	{
		auto loc = glGetUniformLocation(programID, name.c_str());
		uniformLocations[name] = loc;
	}
	intMap[name] = val;
}

void GlslProgram::bindInt(string name, int val)
{
	if (uniformLocations.find(name) == uniformLocations.end())
	{
		auto loc = glGetUniformLocation(programID, name.c_str());
		uniformLocations[name] = loc;
	}
	intMap[name] = val;
	glUniform1i(uniformLocations[name], val);
}

void GlslProgram::bindAllUniforms()
{
	for (auto itr : vec2Map)
	{
		glUniform2fv(uniformLocations[itr.first], 1, &itr.second[0]);
	}

	for (auto itr : vec3Map)
	{
		glUniform3fv(uniformLocations[itr.first], 1, &itr.second[0]);
	}

	for (auto itr : vec4Map)
	{
		glUniform4fv(uniformLocations[itr.first], 1, &itr.second[0]);
	}

	for (auto itr : mat4Map)
	{
		glUniformMatrix4fv(uniformLocations[itr.first], 1, GL_FALSE, &itr.second[0][0]);
	}

	for (auto itr : floatMap)
	{
		glUniform1f(uniformLocations[itr.first], itr.second);
	}

	for (auto itr : intMap)
	{
		glUniform1i(uniformLocations[itr.first], itr.second);
	}
}


GlslProgram::~GlslProgram()
{
	
	if (programID > 0)
	{
		std::cout << "Deleting program "<<programID << '\n';
		glDeleteProgram(programID);
		programID = 0;
	}
	else
	{
		std::cout << "Can't delete program " << programID << '\n';
	}
}