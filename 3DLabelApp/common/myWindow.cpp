#include <iostream>
#include <algorithm>

#include "common/myWindow.h"
#include "common/shader.hpp"


bool MyWindow::Init()
{
	// 1. Initialise GLFW
	if (!glfwInit())
	{
		std::cout << "Failed to initialize GLFW\n" << std::endl;
		return false;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL 

	// 2. create a window
	window = glfwCreateWindow(width, height, "LabelMesh", NULL, NULL);

	if (window == nullptr) {
		std::cout << "Failed to create a window." << std::endl;
		return false;
	}

	glfwMakeContextCurrent(window);

	glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GL_TRUE);	// ensure we can capture the mouse button being pressed 
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);			// ensure we can capture keys being pressed

	// 3. Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		std::cout << "Failed to initialize GLEW\n" << std::endl;
		glfwTerminate();
		return false;
	}

	// 4. Register functions
	glfwSetScrollCallback(window, funcScrollCallback);
	glfwSetDropCallback(window, funcDropCallback);
	glfwSetMouseButtonCallback(window, funcMouseCallback);

	// 5. Initialize openGL
	InitializeGL();

	return true;
}

void MyWindow::Run()
{
	while (!glfwWindowShouldClose(window)) {
		auto mesh = meshes[curVAOIdx];
		glfwGetWindowSize(window, &width, &height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(programID);

		SetMVP();
		SetLight();

		//_check_shader_values();

		// attrib
		glEnableVertexAttribArray(vPositionID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[curVAOIdx]);
		glVertexAttribPointer(vPositionID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(vNormalID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexNormalBuffers[curVAOIdx]);
		glVertexAttribPointer(vNormalID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(vColorID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexColorBuffers[curVAOIdx]);
		glVertexAttribPointer(vColorID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindVertexArray(vaos[0]);
		glDrawElements(GL_TRIANGLES, mesh->nTriangles()*3, GL_UNSIGNED_INT, (void*)0);

		// scroll & mouse event should be after the glDraw
		ScrollEvent();
		MouseEvent();

		// disable
		glDisableVertexAttribArray(vPositionID);
		glDisableVertexAttribArray(vNormalID);
		glDisableVertexAttribArray(vColorID);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void MyWindow::ReadMeshFile(const std::string & inputFile)
{
	std::shared_ptr<MyMesh>& mesh = meshes[0];
	mesh = std::make_shared<MyMesh>();

	if (!mesh->ReadMesh(inputFile)) {
		std::cerr << "reading mesh file error\n";
		return;
	}
	else if (mesh->vertices.cols() == 0) {
		std::cerr << "Error: mesh file error, no vertices!\n";
		return;
	}

	BindMeshVAO(0);
}

void MyWindow::ReadLabelFile(const std::string & fp)
{
	std::string format= fp.substr(fp.find_last_of(".") + 1);

	if (format == "txt"){
		ReadLabelFile_TXT(fp);
	}
	else if (format == "h5") {
		ReadLabelFile_H5(fp);
	}
	else {
		assert(0);
	}
}

void MyWindow::ReadLabelFile_TXT(const std::string & fp)
{
}

void MyWindow::ReadLabelFile_H5(const std::string & h5File)
{
}

void MyWindow::BindMeshVAO(int idx)
{
	const auto& mesh = meshes[idx];

	// 1. vao
	glBindVertexArray(vaos[idx]);

	// 2. buffer data
	// vertices
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[idx]);
	glBufferData(GL_ARRAY_BUFFER, 3 * mesh->nVertices() * sizeof(float), &mesh->vertices(0, 0), GL_STATIC_DRAW);

	// elements
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffers[idx]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * mesh->nTriangles() * sizeof(mesh->triangles(0, 0)), &mesh->triangles(0, 0), GL_STATIC_DRAW);

	// vertex normals
	glBindBuffer(GL_ARRAY_BUFFER, vertexNormalBuffers[idx]);
	glBufferData(GL_ARRAY_BUFFER, 3 * mesh->nVertices() * sizeof(float), mesh->vertex_normals.data(), GL_STATIC_DRAW);

	// vertex colors
	glBindBuffer(GL_ARRAY_BUFFER, vertexColorBuffers[idx]);
	glBufferData(GL_ARRAY_BUFFER, 3 * mesh->nVertices() * sizeof(float), mesh->vertex_colors.data(), GL_STATIC_DRAW);

	// 3. attributes
	// attrib buffer: vertices
	glEnableVertexAttribArray(vPositionID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[idx]);
	glVertexAttribPointer(vPositionID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// attrib buffer: vertex normals
	glEnableVertexAttribArray(vNormalID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexNormalBuffers[idx]);
	glVertexAttribPointer(vNormalID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// attrib buffer: vertex colors
	glEnableVertexAttribArray(vColorID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexColorBuffers[idx]);
	glVertexAttribPointer(vColorID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
}

glm::vec3 MyWindow::TransPixelToModel(double xpos, double ypos) const
{
	GLfloat depth;
	GLint winX = xpos;
	GLint winY = height - (GLint)ypos;
	glReadPixels(winX, winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);		// draw first, otherwise the depth will be always 1.0

	//GLfloat clip_z = (depth - 0.5f)*2.0f;
	//GLfloat world_z = 2.f * near * far / (clip_z * (far - near) - (far + near));

	glm::vec3 winCoords(winX, winY, depth);
	glm::vec4 viewport(0.0f, 0.0f, (float)width, (float)height);
	glm::mat4 modelView = View * Model;
	glm::vec3 unprojected = glm::unProject(winCoords, modelView, Projection, viewport);

	return unprojected;
}

void MyWindow::LabelMesh()
{
	auto mesh = meshes[curVAOIdx];
	Eigen::Matrix4f mvp = GlmToEigen(Projection*View*Model);

	labelTool.Set(mvp, 0, 0, width, height);
	auto triangleLabels = labelTool.CalcTriangleLabels(mesh->vertices, mesh->triangles);
	labelTool.Clear();		// remember clear

	if (triangleLabels.cols() == mesh->triangle_labels.cols())
		mesh->triangle_labels = triangleLabels;
}

void MyWindow::_check_shader_values()
{
	glm::mat4 model, view, projection;
	glGetUniformfv(programID, modelID, &model[0][0]);
	glGetUniformfv(programID, viewID, &view[0][0]);
	glGetUniformfv(programID, projID, &projection[0][0]);
	
	int a = 0;
}

Eigen::Matrix4f MyWindow::GlmToEigen(const glm::mat4 & mat)
{
	Eigen::Matrix4f m;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			m(i, j) = mat[i][j];
		}
	}
	return m;
}

void MyWindow::ClearMesh()
{
	// file info
	meshFile = std::string();
}

void MyWindow::InitializeGL()
{
	/* 1. general */
	glClearColor(1.f, 1.f, 1.0f, 0.0f);
	//glClearDepth(2.0);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_SMOOTH);

	/* 2. shader */
	programID = LoadShaders("vert.glsl", "frag.glsl");
	glUseProgram(programID);

	modelID = glGetUniformLocation(programID, "modelMat");
	viewID = glGetUniformLocation(programID, "viewMat");
	projID = glGetUniformLocation(programID, "projMat");

	lightPosID = glGetUniformLocation(programID, "lightPos");

	vPositionID = glGetAttribLocation(programID, "vPosition");
	vNormalID = glGetAttribLocation(programID, "vNormal");
	vColorID = glGetAttribLocation(programID, "vColor");

	/* 3. vaos & buffers */ 
	glGenVertexArrays(MAX_NUM_OF_MESHES, vaos);
	glGenBuffers(MAX_NUM_OF_MESHES, vertexBuffers);
	glGenBuffers(MAX_NUM_OF_MESHES, elementBuffers);
	glGenBuffers(MAX_NUM_OF_MESHES, vertexColorBuffers);
	glGenBuffers(MAX_NUM_OF_MESHES, vertexNormalBuffers);

	/* 4.  */
	camera = MyCamera(glm::vec3(0, -100, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, -1));

	GL_Initialized = true;
}

void MyWindow::ClearGL()
{
	glDeleteProgram(programID);
	glDeleteBuffers(MAX_NUM_OF_MESHES, vertexBuffers);
	glDeleteBuffers(MAX_NUM_OF_MESHES, elementBuffers);
	glDeleteBuffers(MAX_NUM_OF_MESHES, vertexColorBuffers);
	glDeleteBuffers(MAX_NUM_OF_MESHES, vertexNormalBuffers);
	glDeleteVertexArrays(MAX_NUM_OF_MESHES, vaos);
}

void MyWindow::SetMVP()
{
	Projection = glm::perspective(fov, (float)width / (float)height, near, far);
	View = camera.Mat();
	Model = meshes[curVAOIdx]->ModelMat();
	//glm::mat4 MVP = Projection * View * _modelMat; // Remember, matrix multiplication is the other way around

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &Model[0][0]);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &View[0][0]);
	glUniformMatrix4fv(projID, 1, GL_FALSE, &Projection[0][0]);

}

void MyWindow::SetLight()
{
	glm::vec3 pos = camera.Eye();
	glUniform3f(lightPosID, pos.x, pos.y, pos.z);
}

void MyWindow::ScrollEvent()
{
	if (std::abs(gScrollYOffset) < 0.00001)
		return;

	glm::vec3 dir = camera.Eye() - camera.Center();
	float dist = glm::length(dir);

	float p = 0.1f;
	float offset = gScrollYOffset > 0.f ? (dist - near)*p : -(dist - near)*p / (1 - p);
	camera.Move(offset);

	gScrollYOffset = 0.f;
}

void MyWindow::MouseEvent()
{
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	double dx = xpos - lastCursorPosX;
	double dy = ypos - lastCursorPosY;

	std::shared_ptr<MyMesh> mesh = meshes[curVAOIdx];

	if (gMouseState == GLFW_PRESS) {
		if (gMouseButton == GLFW_MOUSE_BUTTON_LEFT) {
			// left 
			if (gModifierKey == GLFW_MOD_CONTROL) {
				// control + left
				windowState = WINDOW_MOD_LABEL;		// set window state
				labelTool.pushback(xpos, ypos);
			}
			else {
				camera.Rotate(-dx / 500.f, dy / 500.f);
			}
		}
		else if (gMouseButton == GLFW_MOUSE_BUTTON_MIDDLE) {
			// middle, move the object
			glm::vec4 viewport(0, 0, width, height);
			glm::vec3 projected = glm::project(mesh->Position(), View*Model, Projection, viewport);
			mesh->Position() = glm::unProject(projected + glm::vec3(dx, -dy, 0.), View*Model, Projection, viewport);
		}
		else if (gMouseButton == GLFW_MOUSE_BUTTON_RIGHT) {
			// right
		}
		else if (gMouseButton == GLFW_MOUSE_BUTTON_LAST) {
			// last mouse button
		}
		else {}
	}
	else {
		// released
		if (gMouseButton == GLFW_MOUSE_BUTTON_LEFT) {
			// left
			if (windowState == WINDOW_MOD_LABEL) {
				LabelMesh();
				windowState = WINDOW_MOD_DEFAULT;
			}
		}
	}

	// update
	lastCursorPosX = xpos;
	lastCursorPosY = ypos;
}

