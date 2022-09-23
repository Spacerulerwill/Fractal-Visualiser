#pragma once

#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <shader/Shader.h>

struct Vec2 {
	float x;
	float y;
};

class Application
{
private:
	static std::unique_ptr<Application> m_Instance;

	// callbacks
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

	// shaders
	Shader m_mandelbrotShader;
	Shader m_burningshipShader;
	Shader m_tricornShader;
	Shader* p_selectedShader = nullptr;

	static GLFWwindow* m_Window;

	// fractal selection
	int selectedFractal = 0;
	static constexpr unsigned int numFractals = 3;
	const char* fractalOptions[numFractals] = {"Mandelbrot", "Burning Ship", "Tricorn"};

	// fractal properties - uniforms
	Vec2 m_Location  = {0.0f, 0.0f};
	float m_Zoom = 2.0f;
	bool m_juliaMode = false;
	int m_Iterations = 200;

	// other properties - non uniform
	bool m_juliaPaused = false;

	// ui elements
	bool iterationsSlider = false;
	bool juliaModeCheckbox = false;
	bool fractalSelector = false;

	void ProcessInput();
	void CheckUI();

	// uniform locations
	static unsigned int resolutionLoc;
	unsigned int locationLoc = 0;
	static unsigned int mousePosLoc;
	unsigned int juliaModeLoc = 0;
	unsigned int zoomLoc = 0;
	unsigned int iterationsLoc = 0;

	// functions
	template<typename T>
	static inline T LinearInterpolate(int x, int width, T minR, T maxR);

	void UpdateShaderMousePosition();

	
public:
	Application();

	void Run();

	static std::unique_ptr<Application>& GetInstance();
};

