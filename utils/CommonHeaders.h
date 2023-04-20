#pragma once
#include <iostream>
#include <string>
#include <memory>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <chrono>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <fmt/format.h>
#include <tiny_obj_loader.h>

using std::cout;
using std::string;
using std::vector;
using std::map;
using std::array;

const char cendl = '\n';