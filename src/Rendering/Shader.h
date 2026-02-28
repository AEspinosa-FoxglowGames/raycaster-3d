#pragma once
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

inline std::string readFile(const char* path)
{
	std::ifstream file(path);
	if (!file.is_open()) {
		std::cerr << "Failed to open shader: " << path << "\n";
		return "";
	}
	std::stringstream ss;
	ss << file.rdbuf();
	return ss.str();
}

inline unsigned int makeShader(const char* vertPath, const char* fragPath)
{
	std::string vertSrc = readFile(vertPath);
	std::string fragSrc = readFile(fragPath);
	const char* vSrc = vertSrc.c_str();
	const char* fSrc = fragSrc.c_str();

	auto compile = [](const char* src, GLenum type) {
		unsigned int s = glCreateShader(type);
		glShaderSource(s, 1, &src, nullptr);
		glCompileShader(s);
		int ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
		if (!ok) {
			char log[512]; glGetShaderInfoLog(s, 512, nullptr, log);
			std::cerr << "Shader error: " << log << "\n";
		}
		return s;
		};

	unsigned int v = compile(vSrc, GL_VERTEX_SHADER);
	unsigned int f = compile(fSrc, GL_FRAGMENT_SHADER);
	unsigned int p = glCreateProgram();
	glAttachShader(p, v); glAttachShader(p, f);
	glLinkProgram(p);
	glDeleteShader(v); glDeleteShader(f);
	return p;
}