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

	Eigen::Matrix3Xf triangle_centers;

	Eigen::Matrix3Xf dup_vertices;	// duplicates, for render, (n_f * 3, 3)
	Eigen::Matrix3Xf dup_vertex_normals;
	Eigen::Matrix3Xf dup_vertex_colors;
	Eigen::ArrayXi dup_vertex_labels;

public:
	typedef OpenMesh::TriMesh_ArrayKernelT<> _MyMesh;

	MyMesh() {}
	~MyMesh() {}
	
	/** IO */
	bool ReadMesh(const std::string& inputFile);
	bool ReadLabels(const std::string& labelFile);
	bool WriteLabels(const std::string& outFile);

	/** accessor */
	int nVertices() const { return vertices.cols(); }
	int nTriangles() const { return triangles.cols(); }

	inline const glm::vec3& GetTranslation() const { return translation; }
	inline void SetTranslation(const glm::vec3& _t) { translation = _t; }

	inline const glm::vec3& GetScale() const { return scale; }
	inline void SetScale(const glm::vec3& _s) { scale = _s; }

	inline glm::mat4 ModelMat() const { return glm::translate(glm::scale(glm::mat4(1.0), scale), translation); }

	void UpdateVertexLabels();
	void UpdateVertexColors();

	void UpdateTriangleLabelsFromVertexLabels();

	// update dup
	void UpdateDupVertexLabels();
	void UpdateDupVertexColors();

	// max connected components
	Eigen::ArrayXi GetMaxConnectedComponentsTriangles(const Eigen::ArrayXi& triangleLabels, int seedTriIdx);

private:
	std::string meshFilename;

	_MyMesh _mesh;
	glm::vec3 translation;
	glm::vec3 scale = glm::vec3(1., 1., 1.);

	void UpdateAll();
	void UpdateVertices();
	void UpdateTriangles();
	void UpdateTriangleNormals();
	void UpdateVertexNormals();
	void UpdateTriangleCenters();

	// duplicate vertices
	void UpdateDupVertices();
	void UpdateDupVertexNormals();

	// labels
	bool ReadLabelTxt(const std::string& file);
	bool ReadLabelH5(const std::string& file);
};

#endif // !MY_MESH_H
