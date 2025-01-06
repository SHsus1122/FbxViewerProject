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

#pragma region 추가 지식들
// OpenGL Context : OpenGL 상태와 객체들을 관리하는 공간, 각 창(window)은 자신만의 OpenGL Context를 가질 수 있습니다.

#pragma endregion


int main()
{
	// ====================================== [ GLFW 설정 시작 ]
	// 초기화 중과 초기화 후의 오류 체크용
	glfwSetErrorCallback(show_glfw_error);

	// 초기화 체크
	if (!glfwInit()) {
		std::cerr << "GLFW 초기화 실패" << '\n';
		exit(-1);
	}

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
		exit(-1);
	}

	// 지정한 GLFW 창(window) OpegGL Context를 현재 활성 컨텍스트(Current Context)로 만듭니다.
	glfwMakeContextCurrent(window);

	glfwSetWindowSizeCallback(window, window_resized);	// 창의 크기가 변경될 때마다 호출되는 콜백 함수입니다.
	glfwSetKeyCallback(window, key_pressed);			// 창에서 키입력이 있을 때마다 호출되는 콜백 함수입니다.

	// VSync 활성화
	// 스왑 간격(VSync)은 버퍼 스와핑이 일어나기 전까지 기다려야하는 프레임 수(디스플레이의 수직 동기화 타이밍)을 의미합니다.
	// GLFW에서 디폴트 스왑 간격은 0입니다. 즉, 백버퍼가 준비되면 프론트 버퍼가 화면에 출력이 완료되지 않았더라도 스와핑이 이루어집니다.
	// 1로 설정하면 모니터의 헤르츠 설정에 따라 맞춰집니다. (60hz면 60fps로 렌더링 즉, 1초에 60프레임으로 화면에 출력)
	glfwSwapInterval(1);
	// ====================================== [ GLFW 설정 마무리 ]


	// ====================================== [ GLEW 설정 시작 ]
	glewExperimental = GL_TRUE;	// GLEW에서 이용 가능한 모든 확장(Extension)기능들을 활성화

	GLenum err = glewInit();	// Glew 초기화 및 에러 검증
	if (err != GLEW_OK) {
		std::cerr << "GLEW 초기화 실패 " << glewGetErrorString(err) << '\n';
		glfwTerminate();
		exit(-1);
	}

	// glGetString : OpenGL의 버전, 확장들의 목록을 알아낼 때 사용합니다.(여기서는 버전 정보)
	std::cout << glGetString(GL_VERSION) << '\n';

	// 현재 OpenGL Context에서 사용 가능한 확장 정보들의 확인
	int nr_extensions = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &nr_extensions);	// 확장 기능 총 개수

	// 확장 기능들의 이름을 반환
	for (int i = 0; i < nr_extensions; ++i) {
		std::cout << glGetStringi(GL_EXTENSIONS, i) << '\n';
	}

	// 화면을 지울때 사용할 색상을 설정합니다.
	glClearColor(1, 1, 1, 1);

	// 렌더링 루프의 구성 요소들입니다.
	while (!glfwWindowShouldClose(window)) {

		glClear(GL_COLOR_BUFFER_BIT);	// 화면 색상 버퍼를 설정한 색상으로 초기화 합니다.(즉, 이전 프레임의 잔상이 남지 않도록 화면을 지웁니다)

		glfwSwapBuffers(window);		// 더블 버퍼링을 사용해 렌더링된 내용을 화면에 표시합니다.

		glfwPollEvents();				// GLFW의 입력 이벤트 처리를 합니다.(키보드, 마우스 등)
	}

	glfwDestroyWindow(window);			// GLFW 에서 생성한 창을 파괴 및 리소스를 해제합니다.
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
