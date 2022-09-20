#include "Application.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>


#include <core/Window.h>
#include <vertex/IndexBuffer.h>
#include <vertex/VertexArray.h>
#include <vertex/VertexBufferLayout.h>
#include <vertex/VertexBuffer.h>

#include <iostream>

const unsigned int SCREEN_WIDTH = 1280;
const unsigned int SCREEN_HEIGHT = 720;

// static variable definitions
std::unique_ptr<Application> Application::m_Instance = nullptr;
GLFWwindow* Application::m_Window = nullptr;

unsigned int Application::resolutionLoc = 0;
unsigned int Application::mousePosLoc = 0;
Shader* Application::m_Shader = nullptr;

// information for quad that we render the fractal onto
float vertices[] = {
		 1.0f, 1.0f,   // top right
		 1.0f, -1.0f,   // bottom right
		 -1.0f, -1.0f,   // bottom left
		 -1.0f, 1.0f   // top left 
};
unsigned int indices[] = {  // note that we start from 0!
	0, 1, 3,  // first Triangle
	1, 2, 3   // second Triangle
};

Application::Application()
{
    
}

// linear interpolation for pixel coordinate to points in the imaginary plane
template<typename T>
T Application::LinearInterpolate(int x, int width, T minR, T maxR) {
	T range = maxR - minR;
	return x * (range / width) + minR;
}

void Application::Run()
{
	if (!glfwInit()) {
		std::cout << "Failed to intialise GLFW! Aborting..." << std::endl;
		std::exit(-1);
	}

	// initialise window
	// ----------------

	Window::Init("Fractal Visualiser", 1280, 720);
	m_Window = Window::GetWindow();

	// initialise GLAD
	// ---------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialise GLAD! Aborting..." << std::endl;
		std::exit(-1);
	}

	glfwSetFramebufferSizeCallback(m_Window, Application::framebuffer_size_callback);
	glfwSetCursorPosCallback(m_Window, Application::mouse_cursor_pos_callback);
	glfwSetWindowUserPointer(m_Window, this);

	// create fractal shader - get uniform locations
	// ---------------------

	m_Shader = new Shader("res/shaders/fractal.shader");
	int shaderID = m_Shader->GetID();

	resolutionLoc = glGetUniformLocation(shaderID, "resolution");
	locationLoc = glGetUniformLocation(shaderID, "location");
	mousePosLoc = glGetUniformLocation(shaderID, "mousePos");
	juliaModeLoc = glGetUniformLocation(shaderID, "juliaMode");
	zoomLoc = glGetUniformLocation(shaderID, "zoom");
	color1Loc = glGetUniformLocation(shaderID, "color_1");
	color2Loc = glGetUniformLocation(shaderID, "color_2");
	color3Loc = glGetUniformLocation(shaderID, "color_3");
	color4Loc = glGetUniformLocation(shaderID, "color_4");
	iterationsLoc = glGetUniformLocation(shaderID, "iterations");

	// create buffers for rendering the quad
	VertexBufferLayout layout;
	layout.AddAttribute<float>(2);

	VertexBuffer VBO(vertices, sizeof(vertices));
	IndexBuffer EBO(indices, sizeof(indices));

	VertexArray VAO;
	VAO.AddBuffer(VBO, layout);

	VAO.Bind();
	EBO.Bind();

	m_Shader->Bind();
	glUniform2i(resolutionLoc, SCREEN_WIDTH, SCREEN_HEIGHT);

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// application loop
	// ---------------
	while (!glfwWindowShouldClose(m_Window)) {

		// input handling
		ProcessInput();

		// render
		// ------
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// draw quad to render fractal too
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// ImGui menu
		// ----------
		ImGui::SetNextWindowSize({ 0,0 });
		ImGui::Begin("Control Menu");
		iterationsSlider = ImGui::SliderInt("Iterations", &m_Iterations, 0, 10000);
		juliaModeCheckbox = ImGui::Checkbox("Julia Set Mode", &m_juliaMode);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		CheckUI();

		// glfw: swap buffers and poll IO events (key presses, mouse interactions etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(m_Window);
		glfwPollEvents();
	}

	delete m_Shader;
	glfwTerminate();

}
void Application::framebuffer_size_callback(GLFWwindow * window, int width, int height)
{
	glViewport(0, 0, width, height);
	glUniform2i(resolutionLoc, width, height);
}

void Application::mouse_cursor_pos_callback(GLFWwindow* window, double xposIn, double yposIn) {
	Application* ptr = (Application*)glfwGetWindowUserPointer(window);

	int width, height;
	glfwGetWindowSize(m_Window, &width, &height);

	float zoom = ptr->m_Zoom;
	float xLoc = ptr->m_Location.x;
	float yLoc = ptr->m_Location.y;

	float mouseX = static_cast<float>(xposIn);
	float mouseY = static_cast<float>(yposIn);

	float minR = ((-0.5 * width / height) * zoom) + xLoc;
	float maxR = ((0.5 * width / height) * zoom) + xLoc;
	float minI = -0.5 * zoom - yLoc;
	float maxI = 0.5 * zoom - yLoc;

	float xpos = LinearInterpolate(mouseX, width, minR, maxR);
	float ypos = LinearInterpolate(mouseY, height, minI, maxI);

	glUniform2f(mousePosLoc, xpos, ypos);
}

std::unique_ptr<Application>& Application::GetInstance() {
    if (Application::m_Instance == nullptr) {
        Application::m_Instance = std::unique_ptr<Application>(new Application);
    }
    return Application::m_Instance;
}

void Application::ProcessInput()
{
	if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(m_Window, true);

	if (glfwGetKey(m_Window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		m_Location.x -= 0.01f * m_Zoom;
		glUniform2f(locationLoc, m_Location.x, m_Location.y);
	}
	if (glfwGetKey(m_Window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		m_Location.x += 0.01f * m_Zoom;
		glUniform2f(locationLoc, m_Location.x, m_Location.y);
	}
	if (glfwGetKey(m_Window, GLFW_KEY_UP) == GLFW_PRESS) {
		m_Location.y += 0.01f * m_Zoom;
		glUniform2f(locationLoc, m_Location.x, m_Location.y);
	}
	if (glfwGetKey(m_Window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		m_Location.y -= 0.01f * m_Zoom;
		glUniform2f(locationLoc, m_Location.x, m_Location.y);
	}
	if (glfwGetKey(m_Window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
		m_Zoom -= m_Zoom * 0.01f;
		glUniform1f(zoomLoc, m_Zoom);
	}
	if (glfwGetKey(m_Window, GLFW_KEY_MINUS) == GLFW_PRESS) {
		m_Zoom += m_Zoom * 0.01f;
		glUniform1f(zoomLoc, m_Zoom);
	}
}

void Application::CheckUI()
{
	if (iterationsSlider) {
		glUniform1i(iterationsLoc, m_Iterations);
	}
	if (juliaModeCheckbox) {
		glUniform1i(juliaModeLoc, m_juliaMode);
	}
}
