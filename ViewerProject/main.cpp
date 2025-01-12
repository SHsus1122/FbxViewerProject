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

// [ Vertex 구조체 ]
struct Vertex {
	float position[3];
	float normal[3];
	float uv[2];
};

// [ Camera 구조체 ]
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

// VBO(Vertex Buffer Object)생성 및 GLuint 자료형의 고유 ID 지정하면서 VBO 생성
GLuint VAO;		// Vertex Array Object
GLuint VBO;		// Vertex Buffer Object
GLuint EBO;		// Element Buffer Object
GLuint shader;

std::vector<Vertex> vertices;
std::vector<unsigned int> indices;
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);

// 이전 마우스 위치 저장용
double lastX = 400, lastY = 300;
bool firstMouse = true;

// 마우스 버튼 콜백 (클릭 상태 감지)
bool isRightMousePressed = false;

#pragma endregion

#pragma region Utility Functions

// [ 파일 읽기용 함수 ] - 쉐이더 파일을 읽어와 문자열로 반환
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

// [ 쉐이더 추가용 함수 ] - 쉐이더 소스를 컴파일하고 결과를 반환
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

// [ 쉐이더 컴파일 함수 ] - 쉐이더 프로그램 생성 및 컴파일된 쉐이더 연결
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

// [ 쉐이더 생성 ] - 쉐이더 파일을 읽어와 OpenGL 프로그램으로 컴파일
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

// [ 키 입력 ]
void KeyPressed(GLFWwindow* window, int key, int scancode, int action, int mods) {
	float deltaTime = 0.016f; // 60FPS 가정
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

// [ 마우스 이동 ]
void MouseMoved(GLFWwindow* window, double xpos, double ypos) {
	if (!isRightMousePressed) {
		// 우클릭 해제 상태에서는 현재 마우스 위치를 초기값으로 저장만 함
		lastX = xpos;
		lastY = ypos;
		return;
	}

	float xOffset = xpos - lastX;
	float yOffset = lastY - ypos; // Y 좌표는 위쪽이 양수
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

// [ 마우스 입력 ]
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

// [ FBX 준비 ]
void ReadyFbx() {
	// 1. FBX SDK 매니저 생성
	FbxManager* lSdkManager = FbxManager::Create();
	if (!lSdkManager) {
		std::cerr << "Error: Unable to create FBX SDK manager!" << std::endl;
		return;
	}

	// 2. IOSettings 객체 생성
	FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
	lSdkManager->SetIOSettings(ios);

	// 3. Importer 객체 생성
	FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

	// 4. FBX 파일 로드
	if (!lImporter->Initialize("../SampleResource/woodenboxfab/source/Wooden_Box_FAB.fbx", -1, lSdkManager->GetIOSettings())) {
		std::cerr << "Error: Unable to initialize FBX importer!" << std::endl;
		std::cerr << "Error returned: " << lImporter->GetStatus().GetErrorString() << std::endl;
		return;
	}

	// 5. Scene 객체 생성
	FbxScene* lScene = FbxScene::Create(lSdkManager, "Scene");

	// 6. Importer로 Scene 데이터 로드
	lImporter->Import(lScene);

	// 7. 씬의 루트 노드 탐색
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

	// 8. FBX 리소스 해제
	lImporter->Destroy();
	lSdkManager->Destroy();
}

// [ 버텍스 변환 ]
GLfloat* ConvertVertex() {
	// vertices: FBX 데이터를 담고 있는 std::vector<Vertex>
	std::vector<GLfloat> vertexData;

	// FBX 데이터를 GLfloat로 변환하여 std::vector에 추가
	for (const auto& vertex : vertices) {
		vertexData.push_back(static_cast<GLfloat>(vertex.position[0])); // x
		vertexData.push_back(static_cast<GLfloat>(vertex.position[1])); // y
		vertexData.push_back(static_cast<GLfloat>(vertex.position[2])); // z
	}

	// 동적 배열 생성
	GLfloat* result = new GLfloat[vertexData.size()];

	// std::vector 데이터를 동적 배열로 복사
	std::copy(vertexData.begin(), vertexData.end(), result);

	return result;	// 동적 메모리의 포인터를 반환
}

#pragma endregion

#pragma region GLFW Functions

// [ 에러 체킹 ]
void ShowGlfwError(int error, const char* description) {
	std::cerr << "Error: " << description << '\n';
}

// [ 창 사이즈 변경 ]
void WindowResized(GLFWwindow* window, int width, int height) {
	std::cout << "Window resized, new window size: " << width << " x " << height << '\n';

	//glClearColor(0, 0, 1, 1);
	//glClear(GL_COLOR_BUFFER_BIT);
	//glfwSwapBuffers(window);
	glViewport(0, 0, width, height);
}

// [ GLFW 초기화 및 결과 반환 ]
GLFWwindow* GlfwInitialize() {
	// Window를 만들기 전에 필요한 설정 진행
	// Window를 만들거나 OpenGL Context를 생성하기 전에 설정하며 "glfwWindowHint" 함수를 호출해 설정합니다.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);	// OpenGL 버전 3.3 버전으로 설정합니다.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);	// Major(3) + Minor(3) = 3.3 버전

	// 더이상 사용하지 않는 하위 호환 기능들에 대한 에러 처리(하위호환 함수 즉, 이전버전 일부 함수들이 나오면 사용 X)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// 상위 호환성 지원
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// 윈도우 창 생성 및 설정, 가로세로 설정, 창의 이름, 전체화면 여부
	GLFWwindow* window = glfwCreateWindow(
		800, // width
		600, // height
		"OpenGL Example",
		NULL, NULL);

	// 윈도우 창 생성 실패 여부 체크
	if (!window)
	{
		std::cerr << "윈도우 생성 실패" << '\n';
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// 지정한 GLFW 창(window) OpegGL Context를 현재 활성 컨텍스트(Current Context)로 만듭니다.
	glfwMakeContextCurrent(window);
	glfwSetCursorPosCallback(window, MouseMoved);
	glfwSetMouseButtonCallback(window, MouseButtonPressed);
	glfwSetKeyCallback(window, KeyPressed);
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	// 마우스 커서 숨김

	glfwSetWindowSizeCallback(window, WindowResized);	// 창의 크기가 변경될 때마다 호출되는 콜백 함수입니다.
	glfwSetKeyCallback(window, KeyPressed);				// 창에서 키입력이 있을 때마다 호출되는 콜백 함수입니다.

	// VSync 활성화
	// 스왑 간격(VSync)은 버퍼 스와핑이 일어나기 전까지 기다려야하는 프레임 수(디스플레이의 수직 동기화 타이밍)을 의미합니다.
	// GLFW에서 디폴트 스왑 간격은 0입니다. 즉, 백버퍼가 준비되면 프론트 버퍼가 화면에 출력이 완료되지 않았더라도 스와핑이 이루어집니다.
	// 1로 설정하면 모니터의 헤르츠 설정에 따라 맞춰집니다. (60hz면 60fps로 렌더링 즉, 1초에 60프레임으로 화면에 출력)
	glfwSwapInterval(1);

	return window;
}

// [ 렌더링 함수 ]
void Render() {
	GLfloat* vertexData = ConvertVertex();

	// VAO 생성 및 바인딩
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// VBO 생성 및 바인딩
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	// EBO 생성 및 바인딩
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// Vertex Attribute 설정 (위치 정보만 사용)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	// 바인딩 해제
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	delete[] vertexData; // 동적 메모리 해제
}

// [ 테스트용 삼각형 그리기 함수 ]
void CreateTriangle()
{
	// 삼각형 그리기 꼭짓점 생성
	GLfloat vertices[] = {
		-0.5f, -0.5f, 0.0f,  // 왼쪽 아래
		 0.5f, -0.5f, 0.0f,  // 오른쪽 아래
		 0.0f,  0.5f, 0.0f   // 위쪽
	};

	glGenVertexArrays(1, &VAO); // VAO 생성
	glBindVertexArray(VAO);		// VAO를 OpenGL context에 연결(bind)

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// VBO에 담길 데이터와 크기 지정
	// glBufferData는 현재 연결된 버퍼 오브젝트에 데이터를 복사해 담을 수 있게 합니다.
	// 복사하고자 하는 타겟, 복사할 데티어 크기, 복사할 데이터 자체, 그래픽 카드가 복사된 데이터를 어떻게 활용할지에 대한 힌트
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// OpenGL Context 에 연결된 VBO에 복사, 저장 완료 이후 GPU가 어떤 순서로 어떻게 저장되었는지 알려주기 위한 작업
	// 지금 전달한 꼭짓점 정보의 경우에는 9개의 원소를 지닌 단순한 배열입니다.
	// ---------
	// 꼭짓점의 색, 텍스쳐를 입힐 때 사용할 좌표, 노멀(법선) 등 이러한 정보들을 각각 Vertex Attribute라고 합니다.
	// 이에 대한 정보를 GPU가 어떻게 해석할지 정보를 전달하는 함수가 glVertexAttribPointer 입니다.
	// ---------
	// 첫 번째 인자는 VA의 번호입니다. 이를 추후 셰이더에서 VA의 Location이라고도 합니다. 여기서는 0과 1번째의 VA만 구성합니다.
	// 두 번째 인자는 VA의 크기를 받습니다. 이는 요소의 갯수로 좌표는 x, y, z 총 3개 u, v 두 개가 그 크기에 해당합니다.
	// 세 번째 인자는 데이터의 자료형입니다. 모두 GL_FLOAT 형의 데이터 타입 입니다.
	// 네 번째 인자는 데이터를 정규화(Normalize)할 것인지에 대한 여부입니다.
	// 다섯 번째 인자는 stride입니다. 같은 VA번호의 다음 요소까지의 거리 또는 크기를 바이트로 나타낸 값입니다.
	//	- x와 y사이 거리 또는 uv와 다음 uv사이의 거리로 uv도 GL_FLOAT형의 자료 5개를 넘어가야 다음 좌표 또는 uv에 도달하니 아래처럼 합니다.
	//	- 참고로, 모든 값들이 빈 공간 없이 붙어있을 때에는 0으로 전달해도 OpenGL이 알아서 stride를 계산해줍니다.
	// 여섯 번째 인자는 void 포인터형이어야 하며, 해당 VA의 값이 버퍼에서 얼만큼의 offset후에 시작되는지를 바이트값으로 받습니다.
	//	- 즉, 좌표는 버퍼의 첫 시작부터 그 값이 시작되니 0, uv는 3개의 GL_FLOAT를 건너뛰고 시작되니 아래처럼 작성합니다.
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
	glfwSetErrorCallback(ShowGlfwError);	// 초기화 중과 초기화 후의 오류 체크용

	if (!glfwInit()) {						// GLFW 초기화 체크
		std::cerr << "GLFW 초기화 실패" << '\n';
		exit(EXIT_FAILURE);
	}

	GLFWwindow* window = GlfwInitialize();

	// ViewPort 설정 (창 위의 어느 부분에 그림을 그릴지 결정)
	int framebuf_width, framebuf_height;
	glfwGetFramebufferSize(window, &framebuf_width, &framebuf_height);

	// 뷰포트 좌하단 픽셀(x, y)값과 뷰포트의 가로, 세로 크기가 될 영역(여기서는 창을 처음부터 끝까지 다 채우는 기준)
	glViewport(0, 0, framebuf_width, framebuf_height);

	glewExperimental = GL_TRUE;	// GLEW에서 이용 가능한 모든 확장(Extension)기능들을 활성화

	GLenum err = glewInit();	// Glew 초기화 및 에러 검증
	if (err != GLEW_OK) {
		std::cerr << "GLEW 초기화 실패 " << glewGetErrorString(err) << '\n';
		glfwTerminate();
		exit(-1);
	}

	ReadyFbx();
	Render();

	//CreateTriangle();
	CreateShaderProgramFromFiles("shader.vert", "shader.frag");

	std::cout << glGetString(GL_VERSION) << '\n';		// glGetString : OpenGL의 버전, 확장들의 목록을 알아낼 때 사용합니다.(여기서는 버전 정보)
	int nr_extensions = 0;								// 현재 OpenGL Context에서 사용 가능한 확장 정보들의 확인
	glGetIntegerv(GL_NUM_EXTENSIONS, &nr_extensions);	// 확장 기능 총 개수

	for (int i = 0; i < nr_extensions; ++i) {			// 확장 기능들의 이름을 반환
		std::cout << glGetStringi(GL_EXTENSIONS, i) << '\n';
	}

	// 렌더링 루프의 구성 요소들입니다.
	while (!glfwWindowShouldClose(window)) {
		glClearColor(1, 1, 1, 1);		// 기본 배경색 설정(흰색)
		glClear(GL_COLOR_BUFFER_BIT);	// 화면 색상 버퍼를 설정한 색상으로 초기화 합니다.(즉, 이전 프레임의 잔상이 남지 않도록 화면을 지웁니다)

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

		glUseProgram(shader);			// 쉐이더 프로그램 활성화

		// MVP 행렬을 쉐이더로 전달
		GLuint mvpLoc = glGetUniformLocation(shader, "uMVP");
		glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
		glBindVertexArray(VAO);			// VAO 활성화

		glDrawArrays(GL_TRIANGLES, 0, vertices.size());	// 삼각형 그리기
		//glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0); // 인덱스를 기반으로 렌더링

		glBindVertexArray(0);			// VAO 비활성화
		glUseProgram(0);				// 쉐이더 프로그램 비활성화

		glfwSwapBuffers(window);		// 더블 버퍼링을 사용해 렌더링된 내용을 화면에 표시합니다.
		glfwPollEvents();				// GLFW의 입력 이벤트 처리를 합니다.(키보드, 마우스 등)
	}

	glfwDestroyWindow(window);			// GLFW 에서 생성한 창을 파괴 및 리소스를 해제합니다.
	glfwTerminate();

	return 0;
}
