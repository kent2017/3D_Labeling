#ifndef LABEL_TOOL_H
#define LABEL_TOOL_H

#define _PI 3.14159265359

#include "myMesh.h"
#include <vector>

#include <Eigen/Core>

class LabelTool {
public:
	LabelTool() {}

	/** Setting
	@param mvp
	@param x, y, width, height: viewport
	@param maxDepthOffset: faces whose depths are in [0+frontDepth, maxDepthOffset + frontDepth] will be labeled
	*/
	void Set(const Eigen::Matrix4f& mvp, int x, int y, int width, int height, float maxDepthOffset);
	void Clear();

	void pushback(float xpos, float ypos, float depth);

	void AddLabels(MyMesh& mesh);
	Eigen::ArrayXi CalcLabels(const Eigen::Matrix3Xf& points) const;

private:
	Eigen::Matrix3Xf Project(const Eigen::Matrix3Xf& points) const;		// trans object coord to window coord
	Eigen::Matrix3Xf GetViewportCoordFromScreen() const;

	Eigen::ArrayXi IsInsidePolygon(const Eigen::Matrix3Xf& points, const Eigen::Matrix3Xf& poly) const;
	Eigen::ArrayXi IsFront(const Eigen::Matrix3Xf& points, const Eigen::Matrix3Xf& poly, const Eigen::ArrayXi& isInsidePoly) const;

private:
	Eigen::Matrix4f mvp;
	int x, y, width, height;
	float maxDepthOffset;

	std::vector<Eigen::Vector3f> screenCoords;		// screen coordinates

};

#endif // !LABEL_TOOL_H
