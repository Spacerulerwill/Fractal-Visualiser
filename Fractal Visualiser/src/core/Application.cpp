#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "Application.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <core/Window.h>
#include <vertex/IndexBuffer.h>
#include <vertex/VertexArray.h>
#include <vertex/VertexBufferLayout.h>
#include <vertex/VertexBuffer.h>

#include <libpng16/png.h>

#include <iostream>
#include <sstream>

const unsigned int SCREEN_WIDTH = 1280;
const unsigned int SCREEN_HEIGHT = 720;

// static variable definitions
std::unique_ptr<Application> Application::s_Instance = nullptr;
GLFWwindow* Application::p_Window = nullptr;

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
	srand(static_cast<unsigned int> (time(NULL)));

	if (!glfwInit()) {
		std::cout << "Failed to intialise GLFW! Aborting..." << std::endl;
		std::exit(-1);
	}

	// initialise window
	// ----------------
	Window::Init("Fractal Visualiser", 1280, 720);
	p_Window = Window::GetWindow();

	// initialise GLAD
	// ---------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialise GLAD! Aborting..." << std::endl;
		std::exit(-1);
	}

	// GLFW callback functions
	// -----------------------
	glfwSetFramebufferSizeCallback(p_Window, Application::framebuffer_size_callback);
	glfwSetKeyCallback(p_Window, Application::key_callback);
	glfwSetScrollCallback(p_Window, Application::scroll_callback);
	glfwSetWindowUserPointer(p_Window, this);

	// create fractal shader - get uniform locations
	// --------------------
	m_MandelbrotShader = Shader("res/shaders/mandelbrot.shader");
	m_MandelbrotShader.InitShader();

	m_BurningshipShader = Shader("res/shaders/burningship.shader");
	m_BurningshipShader.InitShader();

	m_TricornShader = Shader("res/shaders/tricorn.shader");
	m_TricornShader.InitShader();

	m_MandelbulbShader = Shader("res/shaders/mandelbulb.shader");
	m_MandelbulbShader.InitShader();

	p_SelectedShader = &m_MandelbrotShader;
	p_SelectedShader->Bind();

	int shaderID = p_SelectedShader->GetID();
	m_ResolutionLoc = glGetUniformLocation(shaderID, "resolution");
	m_LocationLoc = glGetUniformLocation(shaderID, "location");
	m_MousePosLoc = glGetUniformLocation(shaderID, "mousePos");
	m_JuliaModeLoc = glGetUniformLocation(shaderID, "juliaMode");
	m_ZoomLoc = glGetUniformLocation(shaderID, "zoom");
	m_IterationsLoc = glGetUniformLocation(shaderID, "iterations");
	m_Color1Loc = glGetUniformLocation(shaderID, "color_1");
	m_Color2Loc = glGetUniformLocation(shaderID, "color_2");
	m_Color3Loc = glGetUniformLocation(shaderID, "color_3");
	m_Color4Loc = glGetUniformLocation(shaderID, "color_4");

	// create buffers for rendering the quad
	VertexBufferLayout layout;
	layout.AddAttribute<float>(2);

	VertexBuffer VBO(vertices, sizeof(vertices));
	IndexBuffer EBO(indices, sizeof(indices));

	VertexArray VAO;
	VAO.AddBuffer(VBO, layout);

	VAO.Bind();
	EBO.Bind();

	glUniform2i(m_ResolutionLoc, SCREEN_WIDTH, SCREEN_HEIGHT);

	// ImGui context creationg and start up
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = NULL; // disable imgui.ini
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(p_Window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// application loop
	// ---------------
	while (!glfwWindowShouldClose(p_Window)) {

		// input handling
		ProcessInput();

		// render
		// ------
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
		// set min and max x and y points in imaginary plain
		glfwGetWindowSize(p_Window, &m_ScreenWidth, &m_ScreenHeight);

		m_MinR = ((-0.5f * m_ScreenWidth / m_ScreenWidth) * m_Zoom) + m_Location.x;
		m_MaxR = ((0.5f * m_ScreenWidth / m_ScreenWidth) * m_Zoom) + m_Location.x;
		m_MinI = -0.5f * m_Zoom - m_Location.y;
		m_MaxI = 0.5f * m_Zoom - m_Location.y;

		// draw quad to render fractal too - main framebuffer
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// ImGui menu
		// ----------

		if (m_shouldRenderGUI) {

			ImGui::SetNextWindowSize({ 0,0 });

			ImGui::Begin("Control Menu");
			m_isFractalSelectorUsed = ImGui::Combo("Fractals", &p_SelectedFractal, m_FractalOptions, c_NumFractals);
			m_isIterationsSliderUsed = ImGui::SliderInt("Iterations", &m_Iterations, 0, 10000);
			m_isJuliaModeCheckboxUsed = ImGui::Checkbox("Julia Set Mode", &m_isJuliaMode);

			m_isColor1SelectorUsed = ImGui::SliderFloat3("Color 1", m_Color1, 0.0f, 1.0f);
			ImGui::SameLine();
			m_isRandomiseColor1ButtonPressed = ImGui::Button("Randomise##Color 1");

			m_isColor2SelectorUsed = ImGui::SliderFloat3("Color 2", m_Color2, 0.0f, 1.0f);
			ImGui::SameLine();
			m_isRandomiseColor2ButtonPressed = ImGui::Button("Randomise##Color 2");

			m_isColor3SelectorUsed = ImGui::SliderFloat3("Color 3", m_Color3, 0.0f, 1.0f);
			ImGui::SameLine();
			m_isRandomiseColor3ButtonPressed = ImGui::Button("Randomise##Color 3");

			m_isColor4SelectorUsed = ImGui::SliderFloat3("Color 4", m_Color4, 0.0f, 1.0f);
			ImGui::SameLine();
			m_isRandomiseColor4ButtonPressed = ImGui::Button("Randomise##Color 4");

			m_isColorPresetSelectorUsed = ImGui::Combo("Color Presets", &m_SelectedColorPreset, m_ColorPresetOptions, c_NumColorPresets);

			m_isRandomiseAllColorsButtonPressed = ImGui::Button("Randomise All Colors");
			ImGui::SameLine();
			m_isSavePresetButtonPressed = ImGui::Button("Save Preset");

			if (m_isJuliaMode) {
				ImGui::Checkbox("Julia Orbit", &m_isJuliaOrbitOn);
				if (m_isJuliaOrbitOn) {
					ImGui::SliderFloat("Orbit Radius", &m_JuliaOrbitRadius, 0.01f, 5.0f);
					ImGui::SliderFloat("Orbit Speed", &m_JuliaOrbitSpeed, 0.1f, 10.0f);
				}
			}

			ImGui::Text
			("Controls: \n\n"
				"Arrow Keys - pan\n"
				"+/- or Scroll wheel - zoom\n"
				"J - Julia set mode toggle\n"
				"F - Julia set pause toggle\n"
				"R - Reset fractal position\n"
				"H - Toggle GUI visibility (useful for screenshots and videos)\n"
				"P - Take a screenshot"
			);
			ImGui::End();
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	
		CheckUI();

		UpdateShaderMousePosition();

		// glfw: swap buffers and poll IO events (key presses, mouse interactions etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(p_Window);
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
			ptr->m_isJuliaMode = !ptr->m_isJuliaMode;
			glUniform1i(ptr->m_JuliaModeLoc, ptr->m_isJuliaMode);
			break;
		}
		case GLFW_KEY_F: {
			ptr->m_isJuliaPaused = !ptr->m_isJuliaPaused;
			break;
		}
		case GLFW_KEY_R: {
			ptr->m_Location.x = 0.0f;
			ptr->m_Location.y = 0.0f;
			glUniform2f(ptr->m_LocationLoc, ptr->m_Location.x, ptr->m_Location.y);
			ptr->m_Zoom = 2.0f;
			glUniform1f(ptr->m_ZoomLoc, ptr->m_Zoom);
			break;
		}
		case GLFW_KEY_P: {
			int width, height;
			glfwGetWindowSize(p_Window, &width, &height);

			uint8_t* pixels = new uint8_t[3 * width * height];

			glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
			
			// create name
			std::stringstream stream; 
			stream << ptr->m_FractalOptions[ptr->p_SelectedFractal];
			if (ptr->m_isJuliaMode)
				stream << " Julia Set";
			stream << " at " << ptr->m_Location.x << " + " << ptr->m_Location.y << "i.png";
			std::string result = stream.str();
			ptr->save_png_libpng(result.c_str(), pixels, width, height);

			break;
		}
		case GLFW_KEY_H: {
			ptr->m_shouldRenderGUI = !ptr->m_shouldRenderGUI;
		}
		}
	}

}

void Application::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	Application* ptr = (Application*)glfwGetWindowUserPointer(window);

	ptr->m_Zoom -= ptr->m_Zoom * 0.1f * static_cast<float>(yoffset);
	glUniform1f(ptr->m_ZoomLoc, ptr->m_Zoom);
}

void Application::framebuffer_size_callback(GLFWwindow * window, int width, int height)
{
	Application* ptr = (Application*)glfwGetWindowUserPointer(window);

	glViewport(0, 0, width, height);
	glUniform2i(ptr->m_ResolutionLoc, width, height);
}

void Application::UpdateShaderMousePosition() {

	if (m_isJuliaMode) {
		if (!m_isJuliaPaused) {

			double mouseX, mouseY;
			glfwGetCursorPos(p_Window, &mouseX, &mouseY);

			m_MouseXPos = LinearInterpolate(static_cast<int>(mouseX), m_ScreenWidth, m_MinR, m_MaxR);
			m_MouseYPos = LinearInterpolate(static_cast<int>(mouseY), m_ScreenHeight, m_MinI, m_MaxI);

			if (m_isJuliaOrbitOn) {			
				float newXPos = m_MouseXPos + sin(m_JuliaOrbitSpeed * static_cast<float>(glfwGetTime())) * m_JuliaOrbitRadius;
				float newYPos = m_MouseYPos + cos(m_JuliaOrbitSpeed * static_cast<float>(glfwGetTime())) * m_JuliaOrbitRadius;
				glUniform2f(m_MousePosLoc, newXPos, newYPos);
			}
			else {
				glUniform2f(m_MousePosLoc, m_MouseXPos, m_MouseYPos);
			}
		}
		else if (m_isJuliaPaused && m_isJuliaOrbitOn) {
			float newXPos = m_MouseXPos + sin(m_JuliaOrbitSpeed * static_cast<float>(glfwGetTime())) * m_JuliaOrbitRadius;
			float newYPos = m_MouseYPos + cos(m_JuliaOrbitSpeed * static_cast<float>(glfwGetTime())) * m_JuliaOrbitRadius;
			glUniform2f(m_MousePosLoc, newXPos, newYPos);
		}
	}
}

std::unique_ptr<Application>& Application::GetInstance() {
    if (Application::s_Instance == nullptr) {
        Application::s_Instance = std::unique_ptr<Application>(new Application);
    }
    return Application::s_Instance;
}

void Application::RandomiseColor1()
{
	float r, g, b;
	r = (float)rand() / RAND_MAX;
	g = (float)rand() / RAND_MAX;
	b = (float)rand() / RAND_MAX;

	m_Color1[0] = r;
	m_Color1[1] = g;
	m_Color1[2] = b;
	glUniform3f(m_Color1Loc, r, g, b);
}

void Application::RandomiseColor2()
{
	float r, g, b;
	r = (float)rand() / RAND_MAX;
	g = (float)rand() / RAND_MAX;
	b = (float)rand() / RAND_MAX;

	m_Color2[0] = r;
	m_Color2[1] = g;
	m_Color2[2] = b;
	glUniform3f(m_Color2Loc, r, g, b);
}
void Application::RandomiseColor3()
{
	float r, g, b;
	r = (float)rand() / RAND_MAX;
	g = (float)rand() / RAND_MAX;
	b = (float)rand() / RAND_MAX;

	m_Color3[0] = r;
	m_Color3[1] = g;
	m_Color3[2] = b;
	glUniform3f(m_Color3Loc, r, g, b);
}
void Application::RandomiseColor4()
{
	float r, g, b;
	r = (float)rand() / RAND_MAX;
	g = (float)rand() / RAND_MAX;
	b = (float)rand() / RAND_MAX;

	m_Color4[0] = r;
	m_Color4[1] = g;
	m_Color4[2] = b;
	glUniform3f(m_Color4Loc, r, g, b);
}
void Application::ProcessInput()
{
	if (glfwGetKey(p_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(p_Window, true);

	if (glfwGetKey(p_Window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		m_Location.x -= 0.01f * m_Zoom;
		glUniform2f(m_LocationLoc, m_Location.x, m_Location.y);
	}
	if (glfwGetKey(p_Window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		m_Location.x += 0.01f * m_Zoom;
		glUniform2f(m_LocationLoc, m_Location.x, m_Location.y);
	}
	if (glfwGetKey(p_Window, GLFW_KEY_UP) == GLFW_PRESS) {
		m_Location.y += 0.01f * m_Zoom;
		glUniform2f(m_LocationLoc, m_Location.x, m_Location.y);
	}
	if (glfwGetKey(p_Window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		m_Location.y -= 0.01f * m_Zoom;
		glUniform2f(m_LocationLoc, m_Location.x, m_Location.y);
	}
	if (glfwGetKey(p_Window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
		m_Zoom -= m_Zoom * 0.01f;
		glUniform1f(m_ZoomLoc, m_Zoom);
	}
	if (glfwGetKey(p_Window, GLFW_KEY_MINUS) == GLFW_PRESS) {
		m_Zoom += m_Zoom * 0.01f;
		glUniform1f(m_ZoomLoc, m_Zoom);
	}
}

void Application::CheckUI()
{
	// menu widgets and unfiorm updates
	if (m_isIterationsSliderUsed) {
		glUniform1i(m_IterationsLoc, m_Iterations);
	}
	if (m_isJuliaModeCheckboxUsed) {
		glUniform1i(m_JuliaModeLoc, m_isJuliaMode);
	}

	// color changes
	if (m_isColor1SelectorUsed) {
		glUniform3f(m_Color1Loc, m_Color1[0], m_Color1[1], m_Color1[2]);
	}
	if (m_isRandomiseColor1ButtonPressed) {
		RandomiseColor1();
	}

	if (m_isColor2SelectorUsed) {
		glUniform3f(m_Color2Loc, m_Color2[0], m_Color2[1], m_Color2[2]);
	}

	if (m_isRandomiseColor2ButtonPressed) {
		RandomiseColor2();
	}

	if (m_isColor3SelectorUsed) {
		glUniform3f(m_Color3Loc, m_Color3[0], m_Color3[1], m_Color3[2]);
	}

	if (m_isRandomiseColor3ButtonPressed) {
		RandomiseColor3();
		std::cout << "hello!";
	}

	if (m_isColor4SelectorUsed) {
		glUniform3f(m_Color4Loc, m_Color4[0], m_Color4[1], m_Color4[2]);
	}
	if (m_isRandomiseColor4ButtonPressed) {
		RandomiseColor4();
	}

	if (m_isColorPresetSelectorUsed) {
		// set new colors
		float* col1 = m_ColorPresets[m_SelectedColorPreset][0];
		m_Color1[0] = col1[0];
		m_Color1[1] = col1[1];
		m_Color1[2] = col1[2];
		glUniform3f(m_Color1Loc, m_Color1[0], m_Color1[1], m_Color1[2]);

		float* col2 = m_ColorPresets[m_SelectedColorPreset][1];
		m_Color2[0] = col2[0];
		m_Color2[1] = col2[1];
		m_Color2[2] = col2[2];
		glUniform3f(m_Color2Loc, m_Color2[0], m_Color2[1], m_Color2[2]);

		float* col3 = m_ColorPresets[m_SelectedColorPreset][2];
		m_Color3[0] = col3[0];
		m_Color3[1] = col3[1];
		m_Color3[2] = col3[2];
		glUniform3f(m_Color3Loc, m_Color3[0], m_Color3[1], m_Color3[2]);

		float* col4 = m_ColorPresets[m_SelectedColorPreset][3];
		m_Color4[0] = col4[0];
		m_Color4[1] = col4[1];
		m_Color4[2] = col4[2];
		glUniform3f(m_Color4Loc, m_Color4[0], m_Color4[1], m_Color4[2]);
	}

	if (m_isRandomiseAllColorsButtonPressed) {
		RandomiseColor1();
		RandomiseColor2();
		RandomiseColor3();
		RandomiseColor4();
	}

	// change selected fractal
	if (m_isFractalSelectorUsed) {
		switch (p_SelectedFractal) {
		case 0: {
			p_SelectedShader = &m_MandelbrotShader;
			break;
		}
		case 1: {
			p_SelectedShader = &m_BurningshipShader;
			break;
		}
		case 2: {
			p_SelectedShader = &m_TricornShader;
			break;
		}
		case 3: {
			p_SelectedShader = &m_MandelbulbShader;
			break;
		}
		}
		p_SelectedShader->Bind();

		int width, height;
		glfwGetWindowSize(p_Window, &width, &height);

		int shaderID = p_SelectedShader->GetID();

		//UPDATE ALL UNIFORMS FOR NEW SHADER
		glUniform2i(m_ResolutionLoc, width, height);
		glUniform2f(m_LocationLoc, m_Location.x, m_Location.y);
		glUniform1f(m_ZoomLoc, m_Zoom);
		glUniform1i(m_JuliaModeLoc, m_isJuliaMode);
		glUniform1i(m_IterationsLoc, m_Iterations);
		glUniform3f(m_Color1Loc, m_Color1[0], m_Color1[1], m_Color1[2]);
		glUniform3f(m_Color2Loc, m_Color2[0], m_Color2[1], m_Color2[2]);
		glUniform3f(m_Color3Loc, m_Color3[0], m_Color3[1], m_Color3[2]);
		glUniform3f(m_Color4Loc, m_Color4[0], m_Color4[1], m_Color4[2]);
	}
}

bool Application::save_png_libpng(const char* filename, uint8_t* pixels, int w, int h)
{
	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png)
		return false;

	png_infop info = png_create_info_struct(png);
	if (!info) {
		png_destroy_write_struct(&png, &info);
		return false;
	}

	FILE* fp = fopen(filename, "wb");
	if (!fp) {
		png_destroy_write_struct(&png, &info);
		return false;
	}

	png_init_io(png, fp);
	png_set_IHDR(png, info, w, h, 8 /* depth */, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_colorp palette = (png_colorp)png_malloc(png, PNG_MAX_PALETTE_LENGTH * sizeof(png_color));
	if (!palette) {
		fclose(fp);
		png_destroy_write_struct(&png, &info);
		return false;
	}
	png_set_PLTE(png, info, palette, PNG_MAX_PALETTE_LENGTH);
	png_write_info(png, info);
	png_set_packing(png);

	png_bytepp rows = (png_bytepp)png_malloc(png, h * sizeof(png_bytep));
	for (int i = 0; i < h; ++i)
		rows[i] = (png_bytep)(pixels + (h - i) * w * 3);

	png_write_image(png, rows);
	png_write_end(png, info);
	png_free(png, palette);
	png_destroy_write_struct(&png, &info);

	fclose(fp);
	delete[] rows;
	return true;
}