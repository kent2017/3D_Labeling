#ifndef CONTROL_H
#define CONTROL_H

#include <vector>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


void funcScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void funcDropCallback(GLFWwindow* window, int path_count, const char* paths[]);		// drag paths onto the window
void funcMouseCallback(GLFWwindow* window, int button, int action, int mods);

#endif
