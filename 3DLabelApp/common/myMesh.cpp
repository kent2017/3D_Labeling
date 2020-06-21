#include "myMesh.h"

bool MyMesh::ReadMesh(const std::string& inputFile) {
	std::cout << "Reading mesh file: " << inputFile << std::endl;
	if (!OpenMesh::IO::read_mesh(_mesh, inputFile))
		return false;
	else {
		std::cout << "# Vertices: " << _mesh.n_vertices() << std::endl;
		std::cout << "# Edges   : " << _mesh.n_edges() << std::endl;
		std::cout << "# Faces   : " << _mesh.n_faces() << std::endl;

		UpdateAll();
		triangle_labels = Eigen::ArrayXi::Zero(triangles.cols());		// initialize labels
		return true;
	}
}

void MyMesh::UpdateAll()
{
	UpdateVertices();
	UpdateTriangles();
	UpdateVertexNormals();
	UpdateTriangleNormals();
}

void MyMesh::UpdateVertices()
{
	Eigen::Matrix3Xf mat(3, _mesh.n_vertices());

	for (int i = 0; i < _mesh.n_vertices();i++) {
		_MyMesh::VertexHandle v_hd = _mesh.vertex_handle(i);
		_MyMesh::Point p = _mesh.point(v_hd);
		mat(0, i) = p[0];
		mat(1, i) = p[1];
		mat(2, i) = p[2];
	}

	vertices = mat;
}

void MyMesh::UpdateTriangles()
{
	Eigen::Matrix3Xi mat(3, _mesh.n_faces());

	for (int i = 0; i < _mesh.n_faces(); i++) {
		_MyMesh::FaceHandle fh = _mesh.face_handle(i);
		_MyMesh::ConstFaceVertexIter cfvit = _mesh.cfv_iter(fh);

		for (int k = 0; k < 3; k++, ++cfvit) {
			assert(cfvit.is_valid());

			mat(k, i) = cfvit->idx();
		}
	}

	triangles = mat;
}

void MyMesh::UpdateTriangleNormals()
{
	Eigen::Matrix3Xf mat(3, _mesh.n_faces());

	_mesh.request_face_normals();
	_mesh.update_normals();

	int i = 0;
	for (_MyMesh::FaceIter f_it = _mesh.faces_begin(); f_it != _mesh.faces_end(); f_it++) {
		_MyMesh::Normal n = _mesh.normal(*f_it);
		mat(0, i) = n[0];
		mat(1, i) = n[1];
		mat(2, i) = n[2];
		i++;
	}

	triangle_normals = mat;
}

void MyMesh::UpdateVertexNormals()
{
	Eigen::Matrix3Xf mat(3, _mesh.n_vertices());

	_mesh.request_vertex_normals();

	_mesh.request_face_normals();
	_mesh.update_normals();

	int i = 0;
	for (_MyMesh::VertexIter v_it = _mesh.vertices_begin(); v_it != _mesh.vertices_end(); ++v_it) {
		_MyMesh::Normal n = _mesh.normal(*v_it);
		mat(0, i) = n[0];
		mat(1, i) = n[1];
		mat(2, i) = n[2];
		i++;
	}

	vertex_normals = mat;
}

void MyMesh::UpdateVertexColors()
{
}

