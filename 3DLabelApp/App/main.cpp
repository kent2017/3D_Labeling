#include <string.h>
#include <iostream>
#include <assert.h>

#include "common/myMesh.h"
#include "common/myWindow.h"


std::string inputFile = "../../src/C01000713595_L.stl";


int main() {
	MyWindow window(1024, 768);
	if (!window.Init()) {
		return -1;
	}

	window.ReadMeshFile(inputFile);
	window.Run();

	return 0;
}
