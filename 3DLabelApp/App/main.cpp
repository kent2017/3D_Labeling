#include <string.h>
#include <iostream>
#include <assert.h>

#include "common/myMesh.h"
#include "common/myWindow.h"


std::string inputFile = "E:/StandardLower.stl";


int main() {
	MyWindow window(1024, 768);
	if (!window.Init()) {
		return -1;
	}

	window.ReadMeshFile(inputFile);
	window.Run();

	return 0;
}
