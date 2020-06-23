#ifndef LABEL_TOOL_H
#define LABEL_TOOL_H

#include <vector>

#include <Eigen/Core>

class LabelTool {
public:
	LabelTool() {}

	void Set(const Eigen::Matrix4f& mvp, int x, int y, int width, int height);
	void Clear();

	void pushback(float xpos, float ypos);

	Eigen::ArrayXi CalcTriangleLabels(const Eigen::Matrix3Xf& vertices, const Eigen::Matrix3Xi& triangles) const;

private:
	Eigen::Matrix3Xf ProjectToViewportCoord(const Eigen::Matrix3Xf& points) const;		// trans object coord to window coord

private:
	Eigen::Matrix4f mvp;
	int x, y, width, height;

	std::vector<Eigen::Vector2f> screenXYs;		// screen coordinates

};

#endif // !LABEL_TOOL_H
