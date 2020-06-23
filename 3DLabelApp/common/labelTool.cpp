#include "labelTool.h"

void LabelTool::Set(const Eigen::Matrix4f & mvp, int x, int y, int width, int height)
{
	this->mvp = mvp;
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
}

void LabelTool::Clear()
{
	screenXYs.clear();
	x = y = width = height = 0;
	mvp = Eigen::Matrix4f::Identity(4, 4);
}

void LabelTool::pushback(float xpos, float ypos)
{
	Eigen::Vector2f p(xpos, ypos);

	if (screenXYs.empty()) {
		screenXYs.push_back(p);
		return;
	}

	auto lastPixel = screenXYs[screenXYs.size() - 1];
	auto v = lastPixel - p;
	if (v.transpose()*v > 8) {
		screenXYs.push_back(p);
	}
}

Eigen::ArrayXi LabelTool::CalcTriangleLabels(const Eigen::Matrix3Xf & vertices, const Eigen::Matrix3Xi & triangles) const
{
	Eigen::Matrix3Xf projCoords = ProjectToViewportCoord(vertices);

	return Eigen::ArrayXi();
}

Eigen::Matrix3Xf LabelTool::ProjectToViewportCoord(const Eigen::Matrix3Xf & points) const
{
	int cols = points.cols();

	Eigen::Matrix4Xf homoCoords(4, cols);
	homoCoords.row(3) = Eigen::VectorXf::Ones(cols);

	for (int i = 0; i < 3; i++)
		homoCoords.row(i) = points.row(i);

	Eigen::Matrix4Xf projected = mvp * homoCoords;
	projected = projected.array().rowwise() / projected.row(3).array();		// x, y are in [-1, 1]

	projected.row(0) = (projected.row(0) * 0.5f).array() + 0.5f;		// now x is in [0, 1]

	return Eigen::Matrix3Xf();
}
