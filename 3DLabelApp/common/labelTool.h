#ifndef LABEL_TOOL_H
#define LABEL_TOOL_H

#define _PI 3.14159265359

#include <vector>

#include <Eigen/Core>

class LabelTool {
public:
	LabelTool() {}

	void Set(const Eigen::Matrix4f& mvp, int x, int y, int width, int height);
	void Clear();

	void pushback(float xpos, float ypos, float depth);

	Eigen::ArrayXi CalcVertexLabels(const Eigen::Matrix3Xf& vertices, const Eigen::Matrix3Xi& triangles) const;

private:
	Eigen::Matrix3Xf ProjectToViewportCoord(const Eigen::Matrix3Xf& points) const;		// trans object coord to window coord
	Eigen::Matrix3Xf GetViewportCoordFromScreen() const;

	Eigen::ArrayXi IsInsidePolygon(const Eigen::Matrix3Xf& points, const Eigen::Matrix3Xf& poly) const;
	Eigen::ArrayXi IsFront(const Eigen::Matrix3Xf& points, const Eigen::Matrix3Xf& poly) const;

private:
	Eigen::Matrix4f mvp;
	int x, y, width, height;

	std::vector<Eigen::Vector3f> screenCoords;		// screen coordinates

};

#endif // !LABEL_TOOL_H
