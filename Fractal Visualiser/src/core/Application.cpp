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

	// GLFW callback functions
	// -----------------------
	glfwSetFramebufferSizeCallback(m_Window, Application::framebuffer_size_callback);
	glfwSetKeyCallback(m_Window, Application::key_callback);
	glfwSetWindowUserPointer(m_Window, this);

	// create fractal shader - get uniform locations
	// --------------------
	m_mandelbrotShader = Shader("res/shaders/mandelbrot.shader");
	m_mandelbrotShader.InitShader();

	m_burningshipShader = Shader("res/shaders/burningship.shader");
	m_burningshipShader.InitShader();

	m_tricornShader = Shader("res/shaders/tricorn.shader");
	m_tricornShader.InitShader();

	p_selectedShader = &m_mandelbrotShader;
	p_selectedShader->Bind();

	int shaderID = p_selectedShader->GetID();
	resolutionLoc = glGetUniformLocation(shaderID, "resolution");
	locationLoc = glGetUniformLocation(shaderID, "location");
	mousePosLoc = glGetUniformLocation(shaderID, "mousePos");
	juliaModeLoc = glGetUniformLocation(shaderID, "juliaMode");
	zoomLoc = glGetUniformLocation(shaderID, "zoom");
	iterationsLoc = glGetUniformLocation(shaderID, "iterations");
	color1Loc = glGetUniformLocation(shaderID, "color_1");
	color2Loc = glGetUniformLocation(shaderID, "color_2");
	color3Loc = glGetUniformLocation(shaderID, "color_3");
	color4Loc = glGetUniformLocation(shaderID, "color_4");

	// create buffers for rendering the quad
	VertexBufferLayout layout;
	layout.AddAttribute<float>(2);

	VertexBuffer VBO(vertices, sizeof(vertices));
	IndexBuffer EBO(indices, sizeof(indices));

	VertexArray VAO;
	VAO.AddBuffer(VBO, layout);

	VAO.Bind();
	EBO.Bind();

	glUniform2i(resolutionLoc, SCREEN_WIDTH, SCREEN_HEIGHT);

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = NULL; // disable imgui.ini
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	float color[] = { 1,2,3 };

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
		fractalSelector = ImGui::Combo("Fractals", &selectedFractal, fractalOptions, numFractals);
		iterationsSlider = ImGui::SliderInt("Iterations", &m_Iterations, 0, 10000);
		juliaModeCheckbox = ImGui::Checkbox("Julia Set Mode", &m_juliaMode);

		color1Selector = ImGui::SliderFloat3("Color 1", m_Color1, 0.0f, 1.0f);
		color2Selector = ImGui::SliderFloat3("Color 2", m_Color2, 0.0f, 1.0f);
		color3Selector = ImGui::SliderFloat3("Color 3", m_Color3, 0.0f, 1.0f);
		color4Selector = ImGui::SliderFloat3("Color 4", m_Color4, 0.0f, 1.0f);

		colorPresetSelector = ImGui::Combo("Color Presets", &selectedColorPreset, colorPresetOptions, numColorPresets);

		ImGui::Text
		("Controls: \n\n"
		"Arrow Keys - pan\n"
		"+/- - zoom\n"
		"J - Julia set mode toggle\n"
		"F - Julia set pause toggle\n"
		"R - Reset fractal position\n"
		);

		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		CheckUI();

		if (m_juliaMode && !m_juliaPaused)
			UpdateShaderMousePosition();

		// glfw: swap buffers and poll IO events (key presses, mouse interactions etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(m_Window);
		glfwPollEvents();
	}

	glfwTerminate();

}
void Application::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Application* ptr = (Application*)glfwGetWindowUserPointer(window);

	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_J: {
			ptr->m_juliaMode = !ptr->m_juliaMode;
			glUniform1i(ptr->juliaModeLoc, ptr->m_juliaMode);
			break;
		}
		case GLFW_KEY_F: {
			ptr->m_juliaPaused = !ptr->m_juliaPaused;
			break;
		}
		case GLFW_KEY_R: {
			ptr->m_Location.x = 0.0f;
			ptr->m_Location.y = 0.0f;
			glUniform2f(ptr->locationLoc, ptr->m_Location.x, ptr->m_Location.y);
			ptr->m_Zoom = 2.0f;
			glUniform1f(ptr->zoomLoc, ptr->m_Zoom);
			break;
		}
		}
	}

}
void Application::framebuffer_size_callback(GLFWwindow * window, int width, int height)
{
	glViewport(0, 0, width, height);
	glUniform2i(resolutionLoc, width, height);
}

void Application::UpdateShaderMousePosition() {
	int width, height;
	glfwGetWindowSize(m_Window, &width, &height);

	double mouseX, mouseY;
	glfwGetCursorPos(m_Window, &mouseX, &mouseY);

	mouseX = static_cast<int>(mouseX);
	mouseY = static_cast<int>(mouseY);

	float minR = ((-0.5 * width / height) * m_Zoom) + m_Location.x;
	float maxR = ((0.5 * width / height) * m_Zoom) + m_Location.x;
	float minI = -0.5 * m_Zoom - m_Location.y;
	float maxI = 0.5 * m_Zoom - m_Location.y;

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
	// menu widgets and unfiorm updates
	if (iterationsSlider) {
		glUniform1i(iterationsLoc, m_Iterations);
	}
	if (juliaModeCheckbox) {
		glUniform1i(juliaModeLoc, m_juliaMode);
	}

	// color changes
	if (color1Selector) {
		glUniform3f(color1Loc, m_Color1[0], m_Color1[1], m_Color1[2]);
	}
	if (color2Selector) {
		glUniform3f(color2Loc, m_Color2[0], m_Color2[1], m_Color2[2]);
	}
	if (color3Selector) {
		glUniform3f(color3Loc, m_Color3[0], m_Color3[1], m_Color3[2]);
	}
	if (color4Selector) {
		glUniform3f(color4Loc, m_Color4[0], m_Color4[1], m_Color4[2]);
	}

	if (colorPresetSelector) {
		// set new colors
		float* col1 = colorPresets[selectedColorPreset][0];
		m_Color1[0] = col1[0];
		m_Color1[1] = col1[1];
		m_Color1[2] = col1[2];
		glUniform3f(color1Loc, m_Color1[0], m_Color1[1], m_Color1[2]);

		float* col2 = colorPresets[selectedColorPreset][1];
		m_Color2[0] = col2[0];
		m_Color2[1] = col2[1];
		m_Color2[2] = col2[2];
		glUniform3f(color2Loc, m_Color2[0], m_Color2[1], m_Color2[2]);

		float* col3 = colorPresets[selectedColorPreset][2];
		m_Color3[0] = col3[0];
		m_Color3[1] = col3[1];
		m_Color3[2] = col3[2];
		glUniform3f(color3Loc, m_Color3[0], m_Color3[1], m_Color3[2]);

		float* col4 = colorPresets[selectedColorPreset][3];
		m_Color4[0] = col4[0];
		m_Color4[1] = col4[1];
		m_Color4[2] = col4[2];
		glUniform3f(color4Loc, m_Color4[0], m_Color4[1], m_Color4[2]);
	}

	// change selected fractal
	if (fractalSelector) {
		switch (selectedFractal) {
		case 0: {
			p_selectedShader = &m_mandelbrotShader;
			break;
		}
		case 1: {
			p_selectedShader = &m_burningshipShader;
			break;
		}
		case 2: {
			p_selectedShader = &m_tricornShader;
			break;
		}
		}
		p_selectedShader->Bind();

		int width, height;
		glfwGetWindowSize(m_Window, &width, &height);

		int shaderID = p_selectedShader->GetID();

		//UPDATE ALL UNIFORMS FOR NEW SHADER
		glUniform2i(resolutionLoc, width, height);
		glUniform2f(locationLoc, m_Location.x, m_Location.y);
		glUniform1f(zoomLoc, m_Zoom);
		glUniform1i(juliaModeLoc, m_juliaMode);
		glUniform1i(iterationsLoc, m_Iterations);
		glUniform3f(color1Loc, m_Color1[0], m_Color1[1], m_Color1[2]);
		glUniform3f(color2Loc, m_Color2[0], m_Color2[1], m_Color2[2]);
		glUniform3f(color3Loc, m_Color3[0], m_Color3[1], m_Color3[2]);
		glUniform3f(color4Loc, m_Color4[0], m_Color4[1], m_Color4[2]);
		

	}
}