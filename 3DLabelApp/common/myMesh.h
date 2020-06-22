#ifndef MY_MESH_H
#define MY_MESH_H

#include <iostream>
#include <string.h>

#include <Eigen/Core>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

#include "common/parameters.h"


class MyMesh {
public:
	Eigen::Matrix3Xf vertices;
	Eigen::Matrix3Xi triangles;
	Eigen::Matrix3Xf triangle_normals;
	Eigen::Matrix3Xf vertex_normals;
	Eigen::Matrix3Xf vertex_colors;
	Eigen::ArrayXi triangle_labels;
	Eigen::ArrayXi vertex_labels;

public:
	typedef OpenMesh::TriMesh_ArrayKernelT<> _MyMesh;

	MyMesh() {}
	~MyMesh() {}
	
	bool ReadMesh(const std::string& inputFile);

	int nVertices() const { return vertices.cols(); }
	int nTriangles() const { return triangles.cols(); }

private:
	_MyMesh _mesh;

	void UpdateAll();
	void UpdateVertices();
	void UpdateTriangles();
	void UpdateTriangleNormals();
	void UpdateVertexNormals();

	// labels
	void UpdateVertexLabels();
};

#endif // !MY_MESH_H
