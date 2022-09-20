#include "Window.h"
#include <iostream>

GLFWwindow* Window::m_Window = nullptr;

void Window::Init(const char* title, int width, int height)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window! Aborting..." << std::endl;
		glfwTerminate();
		std::exit(-1);
	}
	glfwMakeContextCurrent(window);

	m_Window = window;
}
