#include "control.h"
#include "parameters.h"

double gScrollXOffset = 0.f;
double gScrollYOffset = 0.f;

std::vector<std::string> gPaths;

int gMouseButton = 0;		// left, middle, right, etc..
int gModifierKey = 0;		// shift, alt, control, ...
int gMouseState = GLFW_RELEASE;			// press or release

void funcScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	gScrollXOffset = xoffset;
	gScrollYOffset = yoffset;
}

void funcDropCallback(GLFWwindow * window, int path_count, const char * paths[])
{
	gPaths.clear();
	for (int i = 0; i < path_count; i++) {
		gPaths.push_back(std::string(paths[i]));
	}
}

void funcMouseCallback(GLFWwindow * window, int button, int action, int mods)
{
	gMouseButton = button;
	gMouseState = action;
	gModifierKey = mods;
}

