#include "include/GL/glew.h"
#include "include/GLFW/glfw3.h"
#include <iostream>

#include <fbxsdk.h>

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "lib/glew32.lib")
#pragma comment(lib, "lib/glfw3.lib")



void window_resized(GLFWwindow* window, int width, int height);

void key_pressed(GLFWwindow* window, int key, int scancode, int action, int mods);
void show_glfw_error(int error, const char* description);


int main()
{
	// 초기화 중과 초기화 후의 오류 체크용
	glfwSetErrorCallback(show_glfw_error);


	if (!glfwInit()) {
		std::cerr << "GLFW 초기화 실패" << '\n';
		exit(-1);
	}


	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);


	GLFWwindow* window = glfwCreateWindow(
		800, // width
		600, // height
		"OpenGL Example",

		NULL, NULL);
	if (!window)
	{
		std::cerr << "윈도우 생성 실패" << '\n';
		glfwTerminate();
		exit(-1);
	}
		

	glfwMakeContextCurrent(window);


	glfwSetWindowSizeCallback(window, window_resized);
	glfwSetKeyCallback(window, key_pressed);


	glfwSwapInterval(1);


	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		std::cerr << "GLEW 초기화 실패 " << glewGetErrorString(err) << '\n';
		glfwTerminate();
		exit(-1);
	}


	std::cout << glGetString(GL_VERSION) << '\n';


	int nr_extensions = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &nr_extensions);

	for (int i = 0; i < nr_extensions; ++i) {
		std::cout << glGetStringi(GL_EXTENSIONS, i) << '\n';
	}


	glClearColor(0, 0, 1, 1);


	while (!glfwWindowShouldClose(window)) {

		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}


	glfwDestroyWindow(window);


	glfwTerminate();
	return 0;
}

void show_glfw_error(int error, const char* description) {
	std::cerr << "Error: " << description << '\n';
}

void window_resized(GLFWwindow* window, int width, int height) {
	std::cout << "Window resized, new window size: " << width << " x " << height << '\n';

	glClearColor(0, 0, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glfwSwapBuffers(window);
}

void key_pressed(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == 'Q' && action == GLFW_PRESS) {
		glfwTerminate();
		exit(0);
	}
}


/* FBX SDK 기본 테스트 코드
#include <fbxsdk.h>
#include <iostream>

int main(int argc, char** argv) {
	// 1. FBX SDK 매니저 생성
	FbxManager* lSdkManager = FbxManager::Create();
	if (!lSdkManager) {
		std::cerr << "Error: Unable to create FBX SDK manager!" << std::endl;
		return -1;
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
		return -1;
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
			std::cout << "Node Name: " << lChildNode->GetName() << std::endl;
		}
	}

	// 8. FBX 리소스 해제
	lImporter->Destroy();
	lSdkManager->Destroy();

	return 0;
}
*/
