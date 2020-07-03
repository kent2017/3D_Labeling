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
		vertex_labels = Eigen::ArrayXi::Zero(vertices.cols());
		vertex_colors = Eigen::Matrix3Xf(3, vertices.cols());

		UpdateVertexColors();

		dup_vertex_labels = Eigen::ArrayXi::Zero(dup_vertices.cols());
		UpdateDupVertexColors();

		Eigen::Vector3f p = vertices.rowwise().mean();
		translation = -glm::vec3(p(0), p(1), p(2));

		return true;
	}
}

void MyMesh::UpdateAll()
{
	UpdateVertices();
	UpdateTriangles();
	UpdateVertexNormals();
	UpdateTriangleNormals();
	UpdateTriangleCenters();
	
	// dup
	UpdateDupVertices();
	UpdateDupVertexNormals();
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

void MyMesh::UpdateTriangleCenters()
{
	Eigen::Matrix3Xf mat(3, _mesh.n_faces());

	int i = 0;
	for (_MyMesh::FaceIter f_it = _mesh.faces_begin(); f_it != _mesh.faces_end(); f_it++) {
		_MyMesh::Normal c = _mesh.calc_face_centroid(*f_it);
		mat(0, i) = c[0];
		mat(1, i) = c[1];
		mat(2, i) = c[2];
		i++;
	}

	triangle_centers = mat;
}

void MyMesh::UpdateDupVertices()
{
	Eigen::Matrix3Xf v(3, nTriangles()*3);

	int v_idx = 0;
	for (int i = 0; i < nTriangles(); i++) {
		for (int k = 0; k < 3; k++) {
			int _i = triangles(k, i);
			
			v.col(v_idx) = vertices.col(_i);
			v_idx++;
		}
	}

	dup_vertices = v;
}

void MyMesh::UpdateDupVertexNormals()
{
	dup_vertex_normals = Eigen::Matrix3Xf(3, nTriangles() * 3);

	for (int i = 0; i < nTriangles(); i++) {
		dup_vertex_normals.col(3 * i) = triangle_normals.col(i);
		dup_vertex_normals.col(3 * i + 1) = triangle_normals.col(i);
		dup_vertex_normals.col(3 * i + 2) = triangle_normals.col(i);
	}

}

void MyMesh::UpdateVertexLabels()
{
	vertex_labels = Eigen::ArrayXi::Zero(vertices.cols());

	for (int i = 0; i < triangles.cols(); i++) {
		if (triangle_labels(i) == 1) {
			for (int j = 0; j < 3; j++) {
				int v_idx = triangles(j, i);
				vertex_labels(v_idx) = 1;
			}
		}
	}
}

void MyMesh::UpdateVertexColors()
{
	for (int i = 0; i < vertex_colors.cols(); i++) {
		int k = vertex_labels(i);
		vertex_colors(0, i) = COLORS[k][0];
		vertex_colors(1, i) = COLORS[k][1];
		vertex_colors(2, i) = COLORS[k][2];
	}
}

void MyMesh::UpdateTriangleLabelsFromVertexLabels()
{
	// make sure vertex_labels are updated
	triangle_labels = Eigen::ArrayXi(nTriangles());

	for (int i = 0; i < nTriangles(); i++) {
		int cnt = 0;
		for (int j = 0; j < 3; j++) {
			int idx = triangles(j, i);
			if (vertex_labels(idx) == 1)
				cnt++;
		}

		triangle_labels(i) = cnt >= 2 ? 1 : 0;
	}
}

void MyMesh::UpdateDupVertexLabels()
{
	dup_vertex_labels = Eigen::ArrayXi(dup_vertices.cols());
	// make sure triangle labels are updated
	for (int i = 0; i < nTriangles(); i++) {
		dup_vertex_labels(3*i) = triangle_labels(i);
		dup_vertex_labels(3*i+1) = triangle_labels(i);
		dup_vertex_labels(3*i+2) = triangle_labels(i);
	}
}

void MyMesh::UpdateDupVertexColors()
{
	dup_vertex_colors = Eigen::Matrix3Xf(3, dup_vertices.cols());
	for (int i = 0; i < dup_vertices.cols(); i++) {
		int k = dup_vertex_labels(i);
		dup_vertex_colors(0, i) = COLORS[k][0];
		dup_vertex_colors(1, i) = COLORS[k][1];
		dup_vertex_colors(2, i) = COLORS[k][2];
	}
}

Eigen::ArrayXi MyMesh::GetMaxConnectedComponentsTriangles(const Eigen::ArrayXi & triangleLabels, int seedTriIdx) 
{
	Eigen::ArrayXi ret = Eigen::ArrayXi::Zero(triangleLabels.size());
	Eigen::ArrayXi visited = Eigen::ArrayXi::Zero(triangleLabels.size());

	std::vector<int> stack;
	stack.push_back(seedTriIdx);
	visited(seedTriIdx) = 1;

	while (!stack.empty()) {
		int curIdx = stack[0];
		ret[curIdx] = 1;

		stack.erase(stack.begin());
		
		_MyMesh::FaceHandle fh = _mesh.face_handle(curIdx);
		_MyMesh::FaceFaceIter ff_it;
		for (ff_it = _mesh.ff_iter(fh); ff_it.is_valid(); ++ff_it) {
			int idx = ff_it->idx();
			if (ff_it.is_valid() && !visited(idx) && triangleLabels(idx)==1) {
				stack.push_back(idx);
				visited(idx) = 1;
			}
		}
	}

	return ret;
}

