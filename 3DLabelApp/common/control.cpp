#include "control.h"

double gScrollXOffset = 0.f;
double gScrollYOffset = 0.f;

void funcScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	gScrollXOffset = xoffset;
	gScrollYOffset = yoffset;
}

