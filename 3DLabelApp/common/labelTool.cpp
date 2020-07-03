#include "labelTool.h"

void LabelTool::Set(const Eigen::Matrix4f & mvp, int x, int y, int width, int height, float maxDepthOffset)
{
	this->mvp = mvp;
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	this->maxDepthOffset = maxDepthOffset;
}

void LabelTool::Clear()
{
	screenCoords.clear();
	x = y = width = height = 0;
	mvp = Eigen::Matrix4f::Identity(4, 4);
}

void LabelTool::pushback(float xpos, float ypos, float depth)
{
	Eigen::Vector3f p(xpos, ypos, depth);

	if (screenCoords.empty()) {
		screenCoords.push_back(p);
		return;
	}

	auto lastPixel = screenCoords[screenCoords.size() - 1];
	auto v = lastPixel - p;
	if (v(0)*v(0) + v(1)*v(1) > 20) {
		screenCoords.push_back(p);
	}
}

void LabelTool::AddLabels(MyMesh & mesh)
{
	Eigen::ArrayXi triangleLabels = CalcLabels(mesh.triangle_centers);

	// determine the frontmost center
	Eigen::Matrix3Xf projected = Project(mesh.triangle_centers);
	for (int i = 0; i < projected.cols(); i++) {
		if (triangleLabels(i) == 0)
			projected(2, i) = 100000.f;
	}

	int idxMinDepth;
	projected.row(2).minCoeff(&idxMinDepth);

	mesh.triangle_labels = mesh.GetMaxConnectedComponentsTriangles(triangleLabels, idxMinDepth);
	mesh.UpdateVertexLabels();
	mesh.UpdateTriangleLabelsFromVertexLabels();
	mesh.UpdateDupVertexLabels();						// update dup_vertex
}

Eigen::ArrayXi LabelTool::CalcLabels(const Eigen::Matrix3Xf& points) const
{
	Eigen::ArrayXi ret;

	// 1. trans the coords of vertices and pixels to viewport coords
	Eigen::Matrix3Xf projVertices = Project(points);		// viewport coords
	Eigen::Matrix3Xf polygon = GetViewportCoordFromScreen();

	// 2. get the indices of the vertices inside the polygon whose vertices are the pixels
	Eigen::ArrayXi isInside = IsInsidePolygon(projVertices, polygon);

	// 3. get the indices of the vertices on the front side
	Eigen::ArrayXi isFront = IsFront(projVertices, polygon, isInside);

	ret = isInside*isFront;
	return ret;
}

Eigen::Matrix3Xf LabelTool::Project(const Eigen::Matrix3Xf & points) const
{
	int cols = points.cols();

	Eigen::Matrix4Xf homoCoords(4, cols);
	homoCoords.row(3) = Eigen::VectorXf::Ones(cols);

	for (int i = 0; i < 3; i++)
		homoCoords.row(i) = points.row(i);

	Eigen::Matrix4Xf projected = mvp * homoCoords;
	projected = projected.array().rowwise() / projected.row(3).array();		// x, y are in [-1, 1]
	projected = (projected * 0.5f).colwise() + Eigen::Vector4f::Ones()*0.5f;

	projected.row(0) = (projected.row(0)*width).array() + x;		// now x is in [width, height]
	projected.row(1) = (projected.row(1)*height).array() + y;		// now y is in [width, height]

	// return
	Eigen::Matrix3Xf retV(3, cols);
	for (int i = 0; i < 3; i++)
		retV.row(i) = projected.row(i);

	return retV;
}

Eigen::Matrix3Xf LabelTool::GetViewportCoordFromScreen() const
{
	int n = screenCoords.size();
	Eigen::Matrix3Xf retMat(3, n+1);
	
	for (int i = 0; i < n; i++) {
		retMat(0, i) = screenCoords[i](0);
		retMat(1, i) = (float)height - screenCoords[i](1);
		retMat(2, i) = screenCoords[i](2);
	}

	retMat(0, n) = retMat(0, 0);
	retMat(1, n) = retMat(1, 0);
	retMat(2, n) = retMat(2, 0);

	return retMat;
}

Eigen::ArrayXi LabelTool::IsInsidePolygon(const Eigen::Matrix3Xf & points, const Eigen::Matrix3Xf & poly) const
{
	int nv = points.cols();
	Eigen::ArrayXi ret(nv);

	Eigen::Matrix2Xf polyXY(2, poly.cols());
	polyXY.row(0) = poly.row(0);
	polyXY.row(1) = poly.row(1);

	//Eigen::Matrix2Xf polyXY(2, 3);
	//polyXY << 0., 0., 1.,
	//	0., 1., 0.;

	for (int i = 0; i < nv; i++) {
		// outside the viewport
		if (points(0, i) < x || points(0, i) > width || points(1, i) < y || points(1, i) > height)
			ret(i) = 0;
		
		// depth <0 or >1
		if (points(2, i) < 0 || points(2, i) > 1)
			ret(i) = 0;

		// 
		Eigen::Vector2f p(points(0, i), points(1, i));
		Eigen::Matrix2Xf vectors = polyXY.colwise() - p;
		vectors.colwise().normalize();

		Eigen::Matrix2Xf adjVectors(2, vectors.cols());
		for (int i = 0; i < vectors.cols() - 1; i++)
			adjVectors.col(i) = vectors.col(i + 1);
		adjVectors.col(vectors.cols() - 1) = vectors.col(0);

		Eigen::ArrayXf signs = ((vectors.row(0).array() * adjVectors.row(1).array() - vectors.row(1).array()*adjVectors.row(0).array()).array()>0.f).cast<float>();
		signs = signs*2.f - 1.f;
		Eigen::ArrayXf a = (vectors.array() * adjVectors.array()).colwise().sum();
		a = a.max(-1.0);
		a = a.min(1.0);
		Eigen::ArrayXf thetas = signs.array() * Eigen::acos(a);

		float thetaTotal = std::abs(thetas.sum());
		if (std::abs(thetaTotal - 2.f*_PI) < 0.01f)
			ret(i) = 1;
		else
			ret(i) = 0;
	}

	int a = ret.sum();

	return ret;
}

Eigen::ArrayXi LabelTool::IsFront(const Eigen::Matrix3Xf & points, const Eigen::Matrix3Xf & poly, const Eigen::ArrayXi& isInsidePoly) const
{
	// 1. determine the minimum depth of points which are inside the polygon.
	float depthMin = 1000.f;

	for (int i = 0; i < points.cols(); i++) {
		if (isInsidePoly(i) && points(2, i) < depthMin) {
			depthMin = points(2, i);
		}
	}

	float depthMax = depthMin + maxDepthOffset;

	Eigen::ArrayXi ret(points.cols());
	for (int i = 0; i < points.cols(); i++) {
		if (points(2, i) >= depthMin && points(2, i) <= depthMax)
			ret(i) = 1;
		else
			ret(i) = 0;
	}

	return ret;
}
