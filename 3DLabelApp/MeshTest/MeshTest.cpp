#include <iostream>
#include <iterator>
#include <string.h>

#include "common/myMesh.h"

using namespace std;

MyMesh mesh;
std::string inputFile = "E:/StandardLower.stl";

int main()
{
	if (!mesh.ReadMesh(inputFile)){
		std::cerr << "read error\n";
	}

	Eigen::ArrayXi a = Eigen::ArrayXi::Zero(10);
	a(0) = 1;

    return 0;
}

