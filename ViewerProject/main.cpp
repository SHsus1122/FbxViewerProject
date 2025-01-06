#include "include/GLEW/glew.h"
#include "include/GLFW/glfw3.h"
#include <iostream>

#include <fbxsdk.h>

#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "lib/glew32.lib")
#pragma comment(lib, "lib/glfw3.lib")



void window_resized(GLFWwindow* window, int width, int height);

void key_pressed(GLFWwindow* window, int key, int scancode, int action, int mods);
void show_glfw_error(int error, const char* description);

#pragma region �߰� ���ĵ�
// OpenGL Context : OpenGL ���¿� ��ü���� �����ϴ� ����, �� â(window)�� �ڽŸ��� OpenGL Context�� ���� �� �ֽ��ϴ�.

#pragma endregion


int main()
{
	// ====================================== [ GLFW ���� ���� ]
	// �ʱ�ȭ �߰� �ʱ�ȭ ���� ���� üũ��
	glfwSetErrorCallback(show_glfw_error);

	// �ʱ�ȭ üũ
	if (!glfwInit()) {
		std::cerr << "GLFW �ʱ�ȭ ����" << '\n';
		exit(EXIT_FAILURE);
	}

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
		exit(-1);
	}

	// ������ GLFW â(window) OpegGL Context�� ���� Ȱ�� ���ؽ�Ʈ(Current Context)�� ����ϴ�.
	glfwMakeContextCurrent(window);

	glfwSetWindowSizeCallback(window, window_resized);	// â�� ũ�Ⱑ ����� ������ ȣ��Ǵ� �ݹ� �Լ��Դϴ�.
	glfwSetKeyCallback(window, key_pressed);			// â���� Ű�Է��� ���� ������ ȣ��Ǵ� �ݹ� �Լ��Դϴ�.

	// VSync Ȱ��ȭ
	// ���� ����(VSync)�� ���� �������� �Ͼ�� ������ ��ٷ����ϴ� ������ ��(���÷����� ���� ����ȭ Ÿ�̹�)�� �ǹ��մϴ�.
	// GLFW���� ����Ʈ ���� ������ 0�Դϴ�. ��, ����۰� �غ�Ǹ� ����Ʈ ���۰� ȭ�鿡 ����� �Ϸ���� �ʾҴ��� �������� �̷�����ϴ�.
	// 1�� �����ϸ� ������� �츣�� ������ ���� �������ϴ�. (60hz�� 60fps�� ������ ��, 1�ʿ� 60���������� ȭ�鿡 ���)
	glfwSwapInterval(1);

	// ViewPort ����
	// â(��ȭ��)���� ��� �κп� �׸��� �׸��� ����
	int framebuf_width, framebuf_height;
	glfwGetFramebufferSize(window, &framebuf_width, &framebuf_height);
	// ����Ʈ ���ϴ� �ȼ�(x, y)���� ����Ʈ�� ����, ���� ũ�Ⱑ �� ����(���⼭�� â�� ó������ ������ �� ä��� ����)
	glViewport(0, 0, framebuf_width, framebuf_height);
	// ====================================== [ GLFW ���� ������ ]


	// ====================================== [ GLEW ���� ���� ]
	glewExperimental = GL_TRUE;	// GLEW���� �̿� ������ ��� Ȯ��(Extension)��ɵ��� Ȱ��ȭ

	GLenum err = glewInit();	// Glew �ʱ�ȭ �� ���� ����
	if (err != GLEW_OK) {
		std::cerr << "GLEW �ʱ�ȭ ���� " << glewGetErrorString(err) << '\n';
		glfwTerminate();
		exit(-1);
	}


	// �ﰢ�� �׸��� ������ ����
	float vertices[] = {
		-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		 0.0f,  0.5f, 0.0f
	};

	// VBO(Vertex Buffer Object)���� �� GLuint �ڷ����� ���� ID �����ϸ鼭 VBO ����
	GLuint VAO;
	GLuint VBO;

	glGenVertexArrays(1, &VAO); // VAO ����
	glBindVertexArray(VAO); // VAO�� OpenGL context�� ����(bind)

	glGenBuffers(1, &VBO);

	// VBO�� GL_ARRAY_BUFFERŸ�ٿ� ���ε�(����)
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT), (void*)(3 * sizeof(GL_FLOAT)));
	//glEnableVertexAttribArray(1);



	// glGetString : OpenGL�� ����, Ȯ����� ����� �˾Ƴ� �� ����մϴ�.(���⼭�� ���� ����)
	std::cout << glGetString(GL_VERSION) << '\n';

	// ���� OpenGL Context���� ��� ������ Ȯ�� �������� Ȯ��
	int nr_extensions = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &nr_extensions);	// Ȯ�� ��� �� ����

	// Ȯ�� ��ɵ��� �̸��� ��ȯ
	for (int i = 0; i < nr_extensions; ++i) {
		std::cout << glGetStringi(GL_EXTENSIONS, i) << '\n';
	}

	// ȭ���� ���ﶧ ����� ������ �����մϴ�.
	glClearColor(1, 1, 1, 1);

	// ������ ������ ���� ��ҵ��Դϴ�.
	while (!glfwWindowShouldClose(window)) {

		glClear(GL_COLOR_BUFFER_BIT);	// ȭ�� ���� ���۸� ������ �������� �ʱ�ȭ �մϴ�.(��, ���� �������� �ܻ��� ���� �ʵ��� ȭ���� ����ϴ�)

		//glUseProgram(ShaderProgramThatWeWillMakeLater);
		glBindVertexArray(VAO);	// �� �ٷ� ��!
		glDrawArrays(GL_TRIANGLES, 0, 3); // ���� �𸣼ŵ� �˴ϴ�

		glfwSwapBuffers(window);		// ���� ���۸��� ����� �������� ������ ȭ�鿡 ǥ���մϴ�.

		glfwPollEvents();				// GLFW�� �Է� �̺�Ʈ ó���� �մϴ�.(Ű����, ���콺 ��)
	}

	glfwDestroyWindow(window);			// GLFW ���� ������ â�� �ı� �� ���ҽ��� �����մϴ�.
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


/* FBX SDK �⺻ �׽�Ʈ �ڵ�
#include <fbxsdk.h>
#include <iostream>

int main(int argc, char** argv) {
	// 1. FBX SDK �Ŵ��� ����
	FbxManager* lSdkManager = FbxManager::Create();
	if (!lSdkManager) {
		std::cerr << "Error: Unable to create FBX SDK manager!" << std::endl;
		return -1;
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
		return -1;
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
			std::cout << "Node Name: " << lChildNode->GetName() << std::endl;
		}
	}

	// 8. FBX ���ҽ� ����
	lImporter->Destroy();
	lSdkManager->Destroy();

	return 0;
}
*/
