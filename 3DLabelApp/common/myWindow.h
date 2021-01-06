#ifndef MY_WINDOW_H
#define MY_WINDOW_H
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "common/control.h"
#include "common/parameters.h"
#include "common/myMesh.h"
#include "common/myCamera.h"
#include "common/labelTool.h"
#include "common/fileContainer.h"

#define MAX_NUM_OF_MESHES 2


class MyWindow {
public:
	MyWindow(int width, int height) {
		this->width = width;
		this->height = height;
	}
	~MyWindow() {
		glfwTerminate();

		if (GL_Initialized)
			ClearGL();
	}

	bool Init();
	void Run();

	void ReadMeshFile(const std::string& inputFile);
	void WriteLabelFile() const;

private:
	void InitializeGL();
	void ClearGL();
	void ClearMesh();		// clear meshes, labels, etc.

	void SetMVP();	// model-view-projection matrix;
	void SetLight();

	/* events */
	void ScrollEvent();
	void MouseEvent();
	void KeyEvent();
	void DropEvent();

	/** gl */
	void BindMeshVAO(int idxMesh);
	glm::vec3 TransPixelToModel(double xpos, double ypos) const;	// screen coordinates
	 
	void UpdateColors();

	/** editing mode */
	void LabelMesh();
	void DelabelMesh();

	/** helper functions */
	void _check_shader_values();
	Eigen::Matrix4f GlmToEigen(const glm::mat4& mat);

private:
	/* window parameters */
	GLFWwindow* window = nullptr;
	int width, height;
	double lastCursorPosX, lastCursorPosY;

	/* window state */
	int windowState = WINDOW_MOD_DEFAULT;
	bool labelUpdated = false;
	int key = -1;

	/* view & projection */
	float fov = glm::radians(45.f);		// field of view
	float near = 0.3f, far = 500.f;		// near and far plane for clipping objects
	MyCamera camera;

	glm::mat4 Model, View, Projection;

	/* gl variables */
	GLuint programID;	// shader program

	GLuint modelID, viewID, projID;	// the id of mvp matrix
	GLuint vPositionID, vNormalID, vColorID;	// the ids of vPosition, vNormal, vColor in shader
	GLuint lightPosID;

	unsigned int curVAOIdx = 0;	// current idx of the mesh
	// back and front
	GLuint vaos[2];
	//GLuint elementBuffers[2];
	GLuint vertexBuffers[1], vertexColorBuffers[1], vertexNormalBuffers[2];

	/**/
	bool GL_Initialized = false;

	/* mesh */
	std::shared_ptr<MyMesh> meshes[MAX_NUM_OF_MESHES];

	/* file info */
	std::string meshFile;
	FileContainer fileContainer = FileContainer(".stl", ".label");

	/* label tool */
	LabelTool labelTool;

	/* vertices & colors */
	Eigen::Matrix3Xf vertices;		//duplicated
	Eigen::Matrix3Xf v_normals;
	Eigen::Matrix3Xf v_colors;		// 
};

#endif	// MY_WINDOW_H
