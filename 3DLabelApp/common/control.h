#ifndef CONTROL_H
#define CONTROL_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static const glm::vec3 COLORS[] = {
	{0.9, 0.9, 0.9},
	{1.0, 0.7, 0.0}, // yellow
	{0.9, 0.7, 0.6}, // pink
	{0.6, 0.9, 0.5}, // green
	{0.5, 0.7, 0.9}, // blue
	{0.9, 0.6, 0.9}, // purple
	{0.0, 0.8, 0.9},
	{0.6, 0.4, 0.4},
	{0.9, 0.9, 0.3}
};

extern double gScrollXOffset;
extern double gScrollYOffset;

void funcScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

#endif
