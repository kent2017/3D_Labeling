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

	// 5. Initialize openGL
	InitializeGL();

	return true;
}

void MyWindow::Run()
{
	while (!glfwWindowShouldClose(window)) {
		auto mesh = meshes[curVAOIdx];

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(programID);

		ScrollEvent();
		MouseEvent();

		SetMVP();

		//_check_shader_values();

		// attrib
		glEnableVertexAttribArray(vPositionID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[curVAOIdx]);
		glVertexAttribPointer(vPositionID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(vNormalID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexNormalBuffers[curVAOIdx]);
		glVertexAttribPointer(vNormalID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindVertexArray(vaos[0]);
		glDrawElements(GL_TRIANGLES, mesh->nTriangles()*3, GL_UNSIGNED_INT, (void*)0);


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

	// 3. attributes
	// attrib buffer: vertices
	glEnableVertexAttribArray(vPositionID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[idx]);
	glVertexAttribPointer(vPositionID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// attrib buffer: vertex normals
	glEnableVertexAttribArray(vNormalID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexNormalBuffers[idx]);
	glVertexAttribPointer(vNormalID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// 4. unbind buffers
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void MyWindow::_check_shader_values()
{
	glm::mat4 model, view, projection;
	glGetUniformfv(programID, modelID, &model[0][0]);
	glGetUniformfv(programID, viewID, &view[0][0]);
	glGetUniformfv(programID, projID, &projection[0][0]);
	
	int a = 0;
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
	glEnable(GL_SMOOTH);

	/* 2. shader */
	programID = LoadShaders("vert.glsl", "frag.glsl");
	glUseProgram(programID);

	modelID = glGetUniformLocation(programID, "modelMat");
	viewID = glGetUniformLocation(programID, "viewMat");
	projID = glGetUniformLocation(programID, "projMat");

	vPositionID = glGetAttribLocation(programID, "vPosition");
	vNormalID = glGetAttribLocation(programID, "vNormal");

	/* 3. vaos & buffers */ 
	glGenVertexArrays(MAX_NUM_OF_MESHES, vaos);
	glGenBuffers(MAX_NUM_OF_MESHES, vertexBuffers);
	glGenBuffers(MAX_NUM_OF_MESHES, elementBuffers);
	//glGenBuffers(MAX_NUM_OF_MESHES, labelBuffers);
	glGenBuffers(MAX_NUM_OF_MESHES, vertexNormalBuffers);

	/* 4.  */
	pCamera = glm::vec3(0, -100, 0);

	GL_Initialized = true;
}

void MyWindow::ClearGL()
{
	glDeleteProgram(programID);
	glDeleteBuffers(MAX_NUM_OF_MESHES, vertexBuffers);
	glDeleteBuffers(MAX_NUM_OF_MESHES, elementBuffers);
	//glDeleteBuffers(MAX_NUM_OF_MESHES, labelBuffers);
	glDeleteBuffers(MAX_NUM_OF_MESHES, vertexNormalBuffers);
	glDeleteVertexArrays(MAX_NUM_OF_MESHES, vaos);
}

void MyWindow::SetMVP()
{
	// Projection matrix : Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(fov, (float)width / (float)height, near, far);
	// Camera matrix
	glm::mat4 View = glm::lookAt(
		pCamera, 
		pTarget,
		glm::vec3(0, 0, -1)  // Head is up (set to 0,-1,0 to look upside-down)
	);
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.f);
	Model = glm::rotate(Model, rx / 16.f / 80.f, glm::vec3(1., 0., 0.));
	Model = glm::rotate(Model, ry / 16.f / 80.f, glm::vec3(0., 1., 0.));
	Model = glm::rotate(Model, rz / 16.f / 80.f, glm::vec3(0., 0., 1.));
	rx = ry = rz = 0;
	_modelMat = Model * _modelMat;
	// Our ModelViewProjection : multiplication of our 3 matrices
	//glm::mat4 MVP = Projection * View * _modelMat; // Remember, matrix multiplication is the other way around

	glUniformMatrix4fv(modelID, 1, GL_FALSE, &_modelMat[0][0]);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &View[0][0]);
	glUniformMatrix4fv(projID, 1, GL_FALSE, &Projection[0][0]);
}

void MyWindow::ScrollEvent()
{
	if (std::abs(gScrollYOffset) < 0.00001)
		return;

	glm::vec3 dir = pCamera - pTarget;
	float dist = _length(dir);

	float p = 0.1f;
	float offset = gScrollYOffset > 0.f ? (dist - near)*p : -(dist - near)*p / (1 - p);
	pCamera += _normalize(dir) * offset;

	gScrollYOffset = 0.f;
}

void MyWindow::MouseEvent()
{
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	double dx = xpos - lastCursorPosX;
	double dy = ypos - lastCursorPosY;

	// left mouse button
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if (state == GLFW_PRESS) {
		// the left mouse is pressed
		rx -= 8 * dy;
		ry += 8 * dx;
	}

	// middle mouse button
	state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);
	if (state == GLFW_PRESS) {
		int a = dy;
	}

	// update
	lastCursorPosX = xpos;
	lastCursorPosY = ypos;
}

