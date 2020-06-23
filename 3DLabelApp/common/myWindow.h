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
	void ReadLabelFile(const std::string& inputFile);

private:
	void InitializeGL();
	void ClearGL();
	void ClearMesh();		// clear meshes, labels, etc.

	void SetMVP();	// model-view-projection matrix;
	void SetLight();

	/* events */
	void ScrollEvent();
	void MouseEvent();

	/** IO */
	void ReadLabelFile_TXT(const std::string& txtFile);
	void ReadLabelFile_H5(const std::string& h5File);

	/** gl */
	void BindMeshVAO(int idxMesh);
	glm::vec3 TransPixelToModel(double xpos, double ypos) const;	// screen coordinates

	/** editing mode */
	void LabelMesh();

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

	/* view & projection */
	float fov = glm::radians(45.f);		// field of view
	float near = 0.1f, far = 500.f;		// near and far plane for clipping objects
	MyCamera camera;

	glm::mat4 Model, View, Projection;

	/* gl variables */
	GLuint programID;	// shader program

	GLuint modelID, viewID, projID;	// the id of mvp matrix
	GLuint vPositionID, vNormalID, vColorID;	// the ids of vPosition, vNormal, vColor in shader
	GLuint lightPosID;

	unsigned int curVAOIdx = 0;	// current idx of the mesh
	GLuint vaos[MAX_NUM_OF_MESHES];
	GLuint elementBuffers[MAX_NUM_OF_MESHES];
	GLuint vertexBuffers[MAX_NUM_OF_MESHES], vertexColorBuffers[MAX_NUM_OF_MESHES], vertexNormalBuffers[MAX_NUM_OF_MESHES];

	/**/
	bool GL_Initialized = false;

	/* mesh */
	std::shared_ptr<MyMesh> meshes[MAX_NUM_OF_MESHES];

	/* file info */
	std::string meshFile;

	/* label tool */
	LabelTool labelTool;
};

#endif	// MY_WINDOW_H
