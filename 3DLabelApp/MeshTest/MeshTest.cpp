#include <iostream>
#include <iterator>
#include <string.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Eigen/Core>
#include "common/myMesh.h"

using namespace std;

MyMesh mesh;
std::string inputFile = "../../StandardLower.stl";

int main()
{
	if (!mesh.ReadMesh(inputFile)){
		std::cerr << "read error\n";
	}

	Eigen::ArrayXi labels = Eigen::ArrayXi::Zero(mesh.triangles.size());
	for (int i = 0; i < 1000; i++)
		labels(i) = 1;

	mesh.GetMaxConnectedComponentsTriangles(labels, 0);

    return 0;
}

