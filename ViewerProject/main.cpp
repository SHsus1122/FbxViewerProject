#include "include/GLEW/glew.h"
#include "include/GLFW/glfw3.h"
#include "include/GLM/glm/glm.hpp"
#include "include/GLM/glm/gtc/matrix_transform.hpp"
#include "include/GLM/glm/gtc/type_ptr.hpp"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <string>
#include <fbxsdk.h>
#include <vector>

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "lib/glew32.lib")
#pragma comment(lib, "lib/glfw3.lib")


#pragma region Structs & Global Variables

// [ Vertex ����ü ]
struct Vertex {
	float position[3];
	float normal[3];
	float uv[2];
};

// [ Camera ����ü ]
struct Camera {
	glm::vec3 position, front, up, right, worldUp;
	float yaw, pitch, roll, speed, sensitivity;

	Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch)
		: position(startPosition), worldUp(startUp), yaw(startYaw), pitch(startPitch), roll(0.0f),
		speed(10.0f), sensitivity(0.1f) {
		updateVectors();
	}

	void updateVectors() {
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = glm::normalize(front);

		right = glm::normalize(glm::cross(front, worldUp));
		up = glm::normalize(glm::cross(right, front));
	}
};

// VBO(Vertex Buffer Object)���� �� GLuint �ڷ����� ���� ID �����ϸ鼭 VBO ����
GLuint VAO;		// Vertex Array Object
GLuint VBO;		// Vertex Buffer Object
GLuint EBO;		// Element Buffer Object
GLuint shader;

std::vector<Vertex> vertices;
std::vector<unsigned int> indices;
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);

// ���� ���콺 ��ġ �����
double lastX = 400, lastY = 300;
bool firstMouse = true;

// ���콺 ��ư �ݹ� (Ŭ�� ���� ����)
bool isRightMousePressed = false;

#pragma endregion

#pragma region Utility Functions

// [ ���� �б�� �Լ� ] - ���̴� ������ �о�� ���ڿ��� ��ȯ
std::string	ReadFile(const char* filePath)
{
	std::string content;
	std::ifstream fileStream(filePath, std::ios::in);

	while (!fileStream.is_open())
	{
		printf("Failed to read %s file! The file doesn't exist.\n", filePath);
		return "";
	}

	std::string line = "";
	while (!fileStream.eof())
	{
		std::getline(fileStream, line);
		content.append(line + "\n");
	}

	fileStream.close();
	return content;
}

// [ ���̴� �߰��� �Լ� ] - ���̴� �ҽ��� �������ϰ� ����� ��ȯ
GLuint AddShader(const char* shaderCode, GLenum shaderType)
{
	GLuint new_shader = glCreateShader(shaderType);

	const GLchar* code[1];
	code[0] = shaderCode;

	glShaderSource(new_shader, 1, code, NULL);

	GLint result = 0;
	GLchar err_log[1024] = { 0 };

	glCompileShader(new_shader);
	glGetShaderiv(new_shader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(new_shader, sizeof(err_log), NULL, err_log);
		printf("Error compiling the %d shader: '%s'\n", shaderType, err_log);
		return 0;
	}
	return new_shader;
}

// [ ���̴� ������ �Լ� ] - ���̴� ���α׷� ���� �� �����ϵ� ���̴� ����
void CompileShader(const char* vsCode, const char* fsCode)
{
	GLuint vs, fs;

	shader = glCreateProgram();

	if (!shader)
	{
		printf("Error: Cannot create shader program.");
		return;
	}

	vs = AddShader(vsCode, GL_VERTEX_SHADER);
	fs = AddShader(fsCode, GL_FRAGMENT_SHADER);
	glAttachShader(shader, vs);  // Attach shaders to the program for linking process.
	glAttachShader(shader, fs);

	GLint result = 0;
	GLchar err_log[1024] = { 0 };

	glLinkProgram(shader);  // Create executables from shader codes to run on corresponding processors.
	glGetProgramiv(shader, GL_LINK_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(shader, sizeof(err_log), NULL, err_log);
		printf("Error linking program: '%s'\n", err_log);
		return;
	}
}

// [ ���̴� ���� ] - ���̴� ������ �о�� OpenGL ���α׷����� ������
void CreateShaderProgramFromFiles(const char* vsPath, const char* fsPath)
{
	std::string vsFile = ReadFile(vsPath);
	std::string fsFile = ReadFile(fsPath);
	const char* vsCode = vsFile.c_str();
	const char* fsCode = fsFile.c_str();

	CompileShader(vsCode, fsCode);
}

#pragma endregion

#pragma region Input & Callbacks

// [ Ű �Է� ]
void KeyPressed(GLFWwindow* window, int key, int scancode, int action, int mods) {
	float deltaTime = 0.016f; // 60FPS ����
	float velocity = camera.speed * deltaTime;

	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		if (key == GLFW_KEY_W) camera.position += camera.front * velocity;
		if (key == GLFW_KEY_S) camera.position -= camera.front * velocity;
		if (key == GLFW_KEY_A) camera.position -= camera.right * velocity;
		if (key == GLFW_KEY_D) camera.position += camera.right * velocity;
		if (key == GLFW_KEY_Q) camera.position -= camera.up * velocity;
		if (key == GLFW_KEY_E) camera.position += camera.up * velocity;
	}
}

// [ ���콺 �̵� ]
void MouseMoved(GLFWwindow* window, double xpos, double ypos) {
	if (!isRightMousePressed) {
		// ��Ŭ�� ���� ���¿����� ���� ���콺 ��ġ�� �ʱⰪ���� ���常 ��
		lastX = xpos;
		lastY = ypos;
		return;
	}

	float xOffset = xpos - lastX;
	float yOffset = lastY - ypos; // Y ��ǥ�� ������ ���
	lastX = xpos;
	lastY = ypos;

	xOffset *= camera.sensitivity;
	yOffset *= camera.sensitivity;

	camera.yaw += xOffset;
	camera.pitch += yOffset;

	if (camera.pitch > 89.0f) camera.pitch = 89.0f;
	if (camera.pitch < -89.0f) camera.pitch = -89.0f;

	camera.updateVectors();
}

// [ ���콺 �Է� ]
void MouseButtonPressed(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (action == GLFW_PRESS) {
			std::cout << "pressed" << std::endl;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			isRightMousePressed = true;
			//firstMouse = false;
		}
		else if (action == GLFW_RELEASE) {
			std::cout << "released" << std::endl;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			isRightMousePressed = false;
		}
	}
}

#pragma endregion

#pragma region FBX Functions

// [ FBX �غ� ]
void ReadyFbx() {
	// 1. FBX SDK �Ŵ��� ����
	FbxManager* lSdkManager = FbxManager::Create();
	if (!lSdkManager) {
		std::cerr << "Error: Unable to create FBX SDK manager!" << std::endl;
		return;
	}

	// 2. IOSettings ��ü ����
	FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
	lSdkManager->SetIOSettings(ios);

	// 3. Importer ��ü ����
	FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

	// 4. FBX ���� �ε�
	if (!lImporter->Initialize("../SampleResource/woodenboxfab/source/Wooden_Box_FAB.fbx", -1, lSdkManager->GetIOSettings())) {
		std::cerr << "Error: Unable to initialize FBX importer!" << std::endl;
		std::cerr << "Error returned: " << lImporter->GetStatus().GetErrorString() << std::endl;
		return;
	}

	// 5. Scene ��ü ����
	FbxScene* lScene = FbxScene::Create(lSdkManager, "Scene");

	// 6. Importer�� Scene ������ �ε�
	lImporter->Import(lScene);

	// 7. ���� ��Ʈ ��� Ž��
	FbxNode* lRootNode = lScene->GetRootNode();
	if (lRootNode) {
		for (int i = 0; i < lRootNode->GetChildCount(); i++) {
			FbxNode* lChildNode = lRootNode->GetChild(i);
			FbxMesh* lMesh = lChildNode->GetMesh();
			int polygonCount = lMesh->GetPolygonCount();
			FbxVector4* controlPoints = lMesh->GetControlPoints();

			std::cout << "Node Name: " << lChildNode->GetName() << std::endl;
			std::cout << "ControlPoints Count: " << controlPoints->Length() << std::endl;
			std::cout << "Polygon Count: " << polygonCount << std::endl;

			float scaleFactor = 0.01f;

			for (size_t j = 0; j < polygonCount; j++) {
				for (int k = 0; k < 3; k++) {
					int controlPointIndex = lMesh->GetPolygonVertex(j, k);
					FbxVector4 pos = lMesh->GetControlPointAt(controlPointIndex);

					//std::cout << "Vertex Position: " << pos[0] << ", " << pos[1] << ", " << pos[2] << std::endl;

					Vertex vertx;
					vertx.position[0] = pos[0] * scaleFactor;
					vertx.position[1] = pos[1] * scaleFactor;
					vertx.position[2] = pos[2] * scaleFactor;

					vertices.push_back(vertx);

					indices.push_back(vertices.size() - 1);
				}
			}
			std::cout << "Vertex Count For OpenGL: " << vertices.size() << std::endl;
		}
	}

	// 8. FBX ���ҽ� ����
	lImporter->Destroy();
	lSdkManager->Destroy();
}

// [ ���ؽ� ��ȯ ]
GLfloat* ConvertVertex() {
	// vertices: FBX �����͸� ��� �ִ� std::vector<Vertex>
	std::vector<GLfloat> vertexData;

	// FBX �����͸� GLfloat�� ��ȯ�Ͽ� std::vector�� �߰�
	for (const auto& vertex : vertices) {
		vertexData.push_back(static_cast<GLfloat>(vertex.position[0])); // x
		vertexData.push_back(static_cast<GLfloat>(vertex.position[1])); // y
		vertexData.push_back(static_cast<GLfloat>(vertex.position[2])); // z
	}

	// ���� �迭 ����
	GLfloat* result = new GLfloat[vertexData.size()];

	// std::vector �����͸� ���� �迭�� ����
	std::copy(vertexData.begin(), vertexData.end(), result);

	return result;	// ���� �޸��� �����͸� ��ȯ
}

#pragma endregion

#pragma region GLFW Functions

// [ ���� üŷ ]
void ShowGlfwError(int error, const char* description) {
	std::cerr << "Error: " << description << '\n';
}

// [ â ������ ���� ]
void WindowResized(GLFWwindow* window, int width, int height) {
	std::cout << "Window resized, new window size: " << width << " x " << height << '\n';

	//glClearColor(0, 0, 1, 1);
	//glClear(GL_COLOR_BUFFER_BIT);
	//glfwSwapBuffers(window);
	glViewport(0, 0, width, height);
}

// [ GLFW �ʱ�ȭ �� ��� ��ȯ ]
GLFWwindow* GlfwInitialize() {
	// Window�� ����� ���� �ʿ��� ���� ����
	// Window�� ����ų� OpenGL Context�� �����ϱ� ���� �����ϸ� "glfwWindowHint" �Լ��� ȣ���� �����մϴ�.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);	// OpenGL ���� 3.3 �������� �����մϴ�.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);	// Major(3) + Minor(3) = 3.3 ����

	// ���̻� ������� �ʴ� ���� ȣȯ ��ɵ鿡 ���� ���� ó��(����ȣȯ �Լ� ��, �������� �Ϻ� �Լ����� ������ ��� X)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// ���� ȣȯ�� ����
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// ������ â ���� �� ����, ���μ��� ����, â�� �̸�, ��üȭ�� ����
	GLFWwindow* window = glfwCreateWindow(
		800, // width
		600, // height
		"OpenGL Example",
		NULL, NULL);

	// ������ â ���� ���� ���� üũ
	if (!window)
	{
		std::cerr << "������ ���� ����" << '\n';
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// ������ GLFW â(window) OpegGL Context�� ���� Ȱ�� ���ؽ�Ʈ(Current Context)�� ����ϴ�.
	glfwMakeContextCurrent(window);
	glfwSetCursorPosCallback(window, MouseMoved);
	glfwSetMouseButtonCallback(window, MouseButtonPressed);
	glfwSetKeyCallback(window, KeyPressed);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	// ���콺 Ŀ�� ����

	glfwSetWindowSizeCallback(window, WindowResized);	// â�� ũ�Ⱑ ����� ������ ȣ��Ǵ� �ݹ� �Լ��Դϴ�.
	glfwSetKeyCallback(window, KeyPressed);				// â���� Ű�Է��� ���� ������ ȣ��Ǵ� �ݹ� �Լ��Դϴ�.

	// VSync Ȱ��ȭ
	// ���� ����(VSync)�� ���� �������� �Ͼ�� ������ ��ٷ����ϴ� ������ ��(���÷����� ���� ����ȭ Ÿ�̹�)�� �ǹ��մϴ�.
	// GLFW���� ����Ʈ ���� ������ 0�Դϴ�. ��, ����۰� �غ�Ǹ� ����Ʈ ���۰� ȭ�鿡 ����� �Ϸ���� �ʾҴ��� �������� �̷�����ϴ�.
	// 1�� �����ϸ� ������� �츣�� ������ ���� �������ϴ�. (60hz�� 60fps�� ������ ��, 1�ʿ� 60���������� ȭ�鿡 ���)
	glfwSwapInterval(1);

	return window;
}

// [ ������ �Լ� ]
void Render() {
	GLfloat* vertexData = ConvertVertex();

	// VAO ���� �� ���ε�
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// VBO ���� �� ���ε�
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	// EBO ���� �� ���ε�
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// Vertex Attribute ���� (��ġ ������ ���)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	// ���ε� ����
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	delete[] vertexData; // ���� �޸� ����
}

// [ �׽�Ʈ�� �ﰢ�� �׸��� �Լ� ]
void CreateTriangle()
{
	// �ﰢ�� �׸��� ������ ����
	GLfloat vertices[] = {
		-0.5f, -0.5f, 0.0f,  // ���� �Ʒ�
		 0.5f, -0.5f, 0.0f,  // ������ �Ʒ�
		 0.0f,  0.5f, 0.0f   // ����
	};

	glGenVertexArrays(1, &VAO); // VAO ����
	glBindVertexArray(VAO);		// VAO�� OpenGL context�� ����(bind)

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// VBO�� ��� �����Ϳ� ũ�� ����
	// glBufferData�� ���� ����� ���� ������Ʈ�� �����͸� ������ ���� �� �ְ� �մϴ�.
	// �����ϰ��� �ϴ� Ÿ��, ������ ��Ƽ�� ũ��, ������ ������ ��ü, �׷��� ī�尡 ����� �����͸� ��� Ȱ�������� ���� ��Ʈ
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// OpenGL Context �� ����� VBO�� ����, ���� �Ϸ� ���� GPU�� � ������ ��� ����Ǿ����� �˷��ֱ� ���� �۾�
	// ���� ������ ������ ������ ��쿡�� 9���� ���Ҹ� ���� �ܼ��� �迭�Դϴ�.
	// ---------
	// �������� ��, �ؽ��ĸ� ���� �� ����� ��ǥ, ���(����) �� �̷��� �������� ���� Vertex Attribute��� �մϴ�.
	// �̿� ���� ������ GPU�� ��� �ؼ����� ������ �����ϴ� �Լ��� glVertexAttribPointer �Դϴ�.
	// ---------
	// ù ��° ���ڴ� VA�� ��ȣ�Դϴ�. �̸� ���� ���̴����� VA�� Location�̶�� �մϴ�. ���⼭�� 0�� 1��°�� VA�� �����մϴ�.
	// �� ��° ���ڴ� VA�� ũ�⸦ �޽��ϴ�. �̴� ����� ������ ��ǥ�� x, y, z �� 3�� u, v �� ���� �� ũ�⿡ �ش��մϴ�.
	// �� ��° ���ڴ� �������� �ڷ����Դϴ�. ��� GL_FLOAT ���� ������ Ÿ�� �Դϴ�.
	// �� ��° ���ڴ� �����͸� ����ȭ(Normalize)�� �������� ���� �����Դϴ�.
	// �ټ� ��° ���ڴ� stride�Դϴ�. ���� VA��ȣ�� ���� ��ұ����� �Ÿ� �Ǵ� ũ�⸦ ����Ʈ�� ��Ÿ�� ���Դϴ�.
	//	- x�� y���� �Ÿ� �Ǵ� uv�� ���� uv������ �Ÿ��� uv�� GL_FLOAT���� �ڷ� 5���� �Ѿ�� ���� ��ǥ �Ǵ� uv�� �����ϴ� �Ʒ�ó�� �մϴ�.
	//	- �����, ��� ������ �� ���� ���� �پ����� ������ 0���� �����ص� OpenGL�� �˾Ƽ� stride�� ������ݴϴ�.
	// ���� ��° ���ڴ� void ���������̾�� �ϸ�, �ش� VA�� ���� ���ۿ��� ��ŭ�� offset�Ŀ� ���۵Ǵ����� ����Ʈ������ �޽��ϴ�.
	//	- ��, ��ǥ�� ������ ù ���ۺ��� �� ���� ���۵Ǵ� 0, uv�� 3���� GL_FLOAT�� �ǳʶٰ� ���۵Ǵ� �Ʒ�ó�� �ۼ��մϴ�.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)(3 * sizeof(GL_FLOAT)));
	//glEnableVertexAttribArray(1);
}

#pragma endregion

int main()
{
	glfwSetErrorCallback(ShowGlfwError);	// �ʱ�ȭ �߰� �ʱ�ȭ ���� ���� üũ��

	if (!glfwInit()) {						// GLFW �ʱ�ȭ üũ
		std::cerr << "GLFW �ʱ�ȭ ����" << '\n';
		exit(EXIT_FAILURE);
	}

	GLFWwindow* window = GlfwInitialize();

	// ViewPort ���� (â ���� ��� �κп� �׸��� �׸��� ����)
	int framebuf_width, framebuf_height;
	glfwGetFramebufferSize(window, &framebuf_width, &framebuf_height);

	// ����Ʈ ���ϴ� �ȼ�(x, y)���� ����Ʈ�� ����, ���� ũ�Ⱑ �� ����(���⼭�� â�� ó������ ������ �� ä��� ����)
	glViewport(0, 0, framebuf_width, framebuf_height);

	glewExperimental = GL_TRUE;	// GLEW���� �̿� ������ ��� Ȯ��(Extension)��ɵ��� Ȱ��ȭ

	GLenum err = glewInit();	// Glew �ʱ�ȭ �� ���� ����
	if (err != GLEW_OK) {
		std::cerr << "GLEW �ʱ�ȭ ���� " << glewGetErrorString(err) << '\n';
		glfwTerminate();
		exit(-1);
	}

	ReadyFbx();
	Render();

	//CreateTriangle();
	CreateShaderProgramFromFiles("shader.vert", "shader.frag");

	std::cout << glGetString(GL_VERSION) << '\n';		// glGetString : OpenGL�� ����, Ȯ����� ����� �˾Ƴ� �� ����մϴ�.(���⼭�� ���� ����)
	int nr_extensions = 0;								// ���� OpenGL Context���� ��� ������ Ȯ�� �������� Ȯ��
	glGetIntegerv(GL_NUM_EXTENSIONS, &nr_extensions);	// Ȯ�� ��� �� ����

	for (int i = 0; i < nr_extensions; ++i) {			// Ȯ�� ��ɵ��� �̸��� ��ȯ
		std::cout << glGetStringi(GL_EXTENSIONS, i) << '\n';
	}

	// ������ ������ ���� ��ҵ��Դϴ�.
	while (!glfwWindowShouldClose(window)) {
		glClearColor(1, 1, 1, 1);		// �⺻ ���� ����(���)
		glClear(GL_COLOR_BUFFER_BIT);	// ȭ�� ���� ���۸� ������ �������� �ʱ�ȭ �մϴ�.(��, ���� �������� �ܻ��� ���� �ʵ��� ȭ���� ����ϴ�)

		glm::mat4 view = glm::lookAt(
			camera.position,
			camera.position + camera.front,
			camera.up
		);

		glm::mat4 projection = glm::perspective(
			glm::radians(45.0f),
			800.0f / 600.0f,
			0.1f,
			1000.0f
		);

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 mvp = projection * view * model;

		glUseProgram(shader);			// ���̴� ���α׷� Ȱ��ȭ

		// MVP ����� ���̴��� ����
		GLuint mvpLoc = glGetUniformLocation(shader, "uMVP");
		glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
		glBindVertexArray(VAO);			// VAO Ȱ��ȭ

		glDrawArrays(GL_TRIANGLES, 0, vertices.size());	// �ﰢ�� �׸���
		//glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0); // �ε����� ������� ������

		glBindVertexArray(0);			// VAO ��Ȱ��ȭ
		glUseProgram(0);				// ���̴� ���α׷� ��Ȱ��ȭ

		glfwSwapBuffers(window);		// ���� ���۸��� ����� �������� ������ ȭ�鿡 ǥ���մϴ�.
		glfwPollEvents();				// GLFW�� �Է� �̺�Ʈ ó���� �մϴ�.(Ű����, ���콺 ��)
	}

	glfwDestroyWindow(window);			// GLFW ���� ������ â�� �ı� �� ���ҽ��� �����մϴ�.
	glfwTerminate();

	return 0;
}
