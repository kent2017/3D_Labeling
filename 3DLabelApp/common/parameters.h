/*
parameters.h
A head file storing global parameters.
*/
#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <vector>
#include <string>

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

/* mouse parameters */
extern int gMouseButton;		// left, middle, right, etc..
extern int gModifierKey;		// shift, alt, control, ...
extern int gMouseState;			// press or release

/* scroll parameters */
extern double gScrollXOffset;
extern double gScrollYOffset;

/* drop parameters */
extern std::vector<std::string> gPaths;		// the dropped paths on the window

#endif // !PARAMETERS_H
