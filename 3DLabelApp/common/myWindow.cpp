#include <iostream>
#include <fstream>
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
	glfwSetKeyCallback(window, funcKeyCallback);

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

		if (labelUpdated) {
			UpdateColors();
			labelUpdated = false;
		}

		// attrib
		for (int i = 0; i < 2; i++) {

			// This is important for rendering back faces.
			if (i == 0)
				glFrontFace(GL_CCW);
			else
				glFrontFace(GL_CW);

			glBindVertexArray(vaos[i]);

			glEnableVertexAttribArray(vPositionID);
			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[0]);
			glVertexAttribPointer(vPositionID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			glEnableVertexAttribArray(vNormalID);
			glBindBuffer(GL_ARRAY_BUFFER, vertexNormalBuffers[i]);
			glVertexAttribPointer(vNormalID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			glEnableVertexAttribArray(vColorID);
			glBindBuffer(GL_ARRAY_BUFFER, vertexColorBuffers[0]);
			glVertexAttribPointer(vColorID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

			//glDrawElements(GL_TRIANGLES, mesh->nTriangles()*3, GL_UNSIGNED_INT, (void*)0);
			glDrawArrays(GL_TRIANGLES, 0, mesh->dup_vertices.cols());

			glBindVertexArray(0);
		}

		// scroll & mouse event should be after the glDraw
		ScrollEvent();
		MouseEvent();
		KeyEvent();
		DropEvent();

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
	meshFile = inputFile;

	std::shared_ptr<MyMesh>& mesh = meshes[0];

	// reserve MVP
	glm::vec3 translation(0.f);
	glm::vec3 scale(1.0f);
	if (mesh != nullptr) {
		translation = mesh->GetTranslation();
		scale = mesh->GetScale();
	}

	mesh = std::make_shared<MyMesh>();
	mesh->SetTranslation(translation);
	mesh->SetScale(scale);

	if (!mesh->ReadMesh(inputFile)) {
		std::cerr << "reading mesh file error\n";
		return;
	}
	else if (mesh->vertices.cols() == 0) {
		std::cerr << "Error: mesh file error, no vertices!\n";
		return;
	}

	std::string labelFile = inputFile.substr(0, inputFile.find_last_of('.')) + ".label";
	if (mesh->ReadLabels(labelFile)) {
		std::cout << "Read labels successfully" << std::endl;
		mesh->UpdateDupVertexLabels();
		mesh->UpdateDupVertexColors();
	}
	else {
		labelFile = inputFile.substr(0, inputFile.find_last_of('.')) + ".txt";
		if (mesh->ReadLabels(labelFile)) {
			std::cout << "Read labels successfully" << std::endl;
			mesh->UpdateDupVertexLabels();
			mesh->UpdateDupVertexColors();
		}
	}

	BindMeshVAO(0);
}

void MyWindow::WriteLabelFile() const
{
	std::string outFile = meshFile.substr(0, meshFile.find_last_of('.')) + ".label";

	meshes[curVAOIdx]->WriteLabels(outFile);

	std::cout << "Save labels to " << outFile << std::endl;
}

void MyWindow::BindMeshVAO(int idx)
{
	const auto& mesh = meshes[idx];

	// vertices
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, 3 * mesh->dup_vertices.cols() * sizeof(float), &mesh->dup_vertices(0, 0), GL_STATIC_DRAW);

	// vertex colors
	glBindBuffer(GL_ARRAY_BUFFER, vertexColorBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, 3 * mesh->dup_vertex_colors.cols() * sizeof(float), mesh->dup_vertex_colors.data(), GL_STATIC_DRAW);

	for (int i = 0; i < 2; i++) {
		// 1. vao
		glBindVertexArray(vaos[i]);

		// 2. buffer data

		// elements
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffers[0]);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * mesh->nTriangles() * sizeof(mesh->triangles(0, 0)), &mesh->triangles(0, 0), GL_STATIC_DRAW);

		// vertex normals
		glBindBuffer(GL_ARRAY_BUFFER, vertexNormalBuffers[i]);
		if (i == 0)
			// front
			glBufferData(GL_ARRAY_BUFFER, 3 * mesh->dup_vertex_normals.cols() * sizeof(float), mesh->dup_vertex_normals.data(), GL_STATIC_DRAW);
		else {
			// back
			Eigen::Matrix3Xf vNormalsBack = mesh->dup_vertex_normals * -1.f;
			glBufferData(GL_ARRAY_BUFFER, 3 * mesh->dup_vertex_normals.cols() * sizeof(float), vNormalsBack.data(), GL_STATIC_DRAW);
		}

		// 3. attributes
		// attrib buffer: vertices
		//glEnableVertexAttribArray(vPositionID);
		//glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[0]);
		//glVertexAttribPointer(vPositionID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//// attrib buffer: vertex normals
		//glEnableVertexAttribArray(vNormalID);
		//glBindBuffer(GL_ARRAY_BUFFER, vertexNormalBuffers[i]);
		//glVertexAttribPointer(vNormalID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//// attrib buffer: vertex colors
		//glEnableVertexAttribArray(vColorID);
		//glBindBuffer(GL_ARRAY_BUFFER, vertexColorBuffers[0]);
		//glVertexAttribPointer(vColorID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	}
}

glm::vec3 MyWindow::TransPixelToModel(double xpos, double ypos) const
{
	GLfloat depth;			// depth is in [0, 1]
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

void MyWindow::UpdateColors()
{
	auto mesh = meshes[curVAOIdx];
	//mesh->UpdateVertexColors();
	mesh->UpdateDupVertexColors();
	glBindBuffer(GL_ARRAY_BUFFER, vertexColorBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, 3 * mesh->dup_vertex_colors.cols() * sizeof(float), mesh->dup_vertex_colors.data(), GL_STATIC_DRAW);
}

void MyWindow::LabelMesh()
{
	auto mesh = meshes[curVAOIdx];
	Eigen::Matrix4f mvp = GlmToEigen(Projection*View*Model);

	// determine the maxDepthOffset
	glm::vec3 dir = glm::normalize(camera.Center() - camera.Eye());
	float length = 2.5f * mesh->GetScale()[0];
	glm::vec3 refP = camera.Eye() + dir *length;
	float depth1 = glm::project(refP, View/**Model*/, Projection, glm::vec4(0, 0, width, height))[2];
	float depth2 = glm::project(refP + dir*length, View/**Model*/, Projection, glm::vec4(0, 0, width, height))[2];

	//float maxDepthOffset = depth2 - depth1;
	float maxDepthOffset = 0.3;

	labelTool.Set(mvp, 0, 0, width, height, maxDepthOffset);
	labelTool.AddLabels(*mesh);
	labelTool.Clear();		// remember clear
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
			m(i, j) = mat[j][i];
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
	glCullFace(GL_BACK);
	//glEnable(GL_SMOOTH);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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
	glGenVertexArrays(2, vaos);
	glGenBuffers(1, vertexBuffers);
	//glGenBuffers(MAX_NUM_OF_MESHES, elementBuffers);
	glGenBuffers(1, vertexColorBuffers);
	glGenBuffers(2, vertexNormalBuffers);

	/* 4.  */
	camera = MyCamera(glm::vec3(0, -100, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, -1));

	GL_Initialized = true;
}

void MyWindow::ClearGL()
{
	glDeleteProgram(programID);
	glDeleteBuffers(1, vertexBuffers);
	//glDeleteBuffers(MAX_NUM_OF_MESHES, elementBuffers);
	glDeleteBuffers(1, vertexColorBuffers);
	glDeleteBuffers(2, vertexNormalBuffers);
	glDeleteVertexArrays(2, vaos);
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

	float p = std::min(0.15f, meshes[curVAOIdx]->GetScale()[0]*0.25f);
	glm::vec3 scale = meshes[curVAOIdx]->GetScale() + glm::vec3(gScrollYOffset > 0.f ? -p : p);
	meshes[curVAOIdx]->SetScale(scale);
	//meshes[curVAOIdx]->Scale() += gScrollYOffset > 0.f ? -p : p;

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

				// get depth
				GLfloat depth;
				GLint winX = xpos;
				GLint winY = height - (GLint)ypos;
				glReadPixels(winX, winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);		// draw first, otherwise the depth will be always 1.0

				labelTool.pushback(xpos, ypos, depth);
			}
			else {
				camera.Rotate(-dx / 500.f, dy / 500.f);
			}
		}
		else if (gMouseButton == GLFW_MOUSE_BUTTON_MIDDLE) {
			// middle, move the object
			glm::vec4 viewport(0, 0, width, height);
			glm::vec3 projected = glm::project(-mesh->GetTranslation(), View*Model, Projection, viewport);

			mesh->SetTranslation(-glm::unProject(projected + glm::vec3(-dx, dy, 0.), View*Model, Projection, viewport));
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
		}
	}

	// update
	lastCursorPosX = xpos;
	lastCursorPosY = ypos;
}

void MyWindow::KeyEvent()
{
	if (gKeyState == GLFW_PRESS) {
		switch (gKeyMods) {
			// control
		case GLFW_MOD_CONTROL:
			switch (gKey) {
			case GLFW_KEY_S:		// control + S
				WriteLabelFile();
				break;
			}
			break;

			// alt
		case GLFW_MOD_ALT:
			break;

			// shift
		case GLFW_MOD_SHIFT:
			break;

		default:
			switch (gKey) {
			case GLFW_KEY_SPACE:	// space, next
				if (fileContainer.size() > 0) {
					std::string file = fileContainer.Pop();
					ReadMeshFile(file);
				}
				break;
			}
		}
	}
	else if (gKeyState == GLFW_RELEASE) {
		// release
		if (gKey == GLFW_KEY_LEFT_CONTROL && windowState == WINDOW_MOD_LABEL) {
			LabelMesh();
			labelUpdated = true;
			windowState = WINDOW_MOD_DEFAULT;
		}
	}
	else {

	}
}

void MyWindow::DropEvent()
{
	if (!gDroppedPaths.empty()) {

		fileContainer.Clear();
		fileContainer.Init(gDroppedPaths);
		if (fileContainer.size() > 0) {
			ReadMeshFile(fileContainer.Pop());
		}

		gDroppedPaths.clear();
	}
}
