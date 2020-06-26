#ifndef MY_MESH_H
#define MY_MESH_H

#include <iostream>
#include <string.h>

#include <Eigen/Core>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "common/parameters.h"


class MyMesh {
public:
	Eigen::Matrix3Xf vertices;	// no duplicates
	Eigen::Matrix3Xi triangles;
	Eigen::Matrix3Xf triangle_normals;
	Eigen::Matrix3Xf vertex_normals;
	Eigen::Matrix3Xf vertex_colors;
	Eigen::ArrayXi triangle_labels;
	Eigen::ArrayXi vertex_labels;

	Eigen::Matrix3Xf dup_vertices;	// duplicates, for render, (n_f * 3, 3)
	Eigen::Matrix3Xf dup_vertex_normals;
	Eigen::Matrix3Xf dup_vertex_colors;
	Eigen::ArrayXi dup_vertex_labels;

public:
	typedef OpenMesh::TriMesh_ArrayKernelT<> _MyMesh;

	MyMesh() {}
	~MyMesh() {}
	
	bool ReadMesh(const std::string& inputFile);

	int nVertices() const { return vertices.cols(); }
	int nTriangles() const { return triangles.cols(); }

	inline const glm::vec3& Translation() const { return translation; }
	inline glm::vec3& Translation() { return translation; }

	inline const glm::vec3& Scale() const { return scale; }
	inline glm::vec3& Scale() { return scale; }

	inline glm::mat4 ModelMat() const { return glm::translate(glm::scale(glm::mat4(1.0), scale), translation); }

	void UpdateVertexLabels();
	void UpdateVertexColors();

	void UpdateTriangleLabelsFromVertexLabels();

	// update dup
	void UpdateDupVertexLabels();
	void UpdateDupVertexColors();

	// max connected components
	Eigen::ArrayXi GetMaxConnectedComponentsTriangles(const Eigen::ArrayXi& triangleLabels, int seedTriIdx) const;

private:
	_MyMesh _mesh;
	glm::vec3 translation;
	glm::vec3 scale = glm::vec3(1., 1., 1.);

	void UpdateAll();
	void UpdateVertices();
	void UpdateTriangles();
	void UpdateTriangleNormals();
	void UpdateVertexNormals();

	// duplicate vertices
	void UpdateDupVertices();
	void UpdateDupVertexNormals();

	// labels
};

#endif // !MY_MESH_H
