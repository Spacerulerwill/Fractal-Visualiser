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
	static void mouse_cursor_pos_callback(GLFWwindow* window, double xposIn, double yposIn);

	static Shader* m_Shader;
	static GLFWwindow* m_Window;

	// fractal properties
	Vec2 m_Location  = {0.0f, 0.0f};
	float m_Zoom = 2.0f;
	bool m_juliaMode = false;
	int m_Iterations = 200;


	// sliders
	bool iterationsSlider;
	bool juliaModeCheckbox;

	void ProcessInput();
	void CheckUI();

	// uniform locations
	static unsigned int resolutionLoc;
	unsigned int locationLoc = 0;
	static unsigned int mousePosLoc;
	unsigned int juliaModeLoc = 0;
	unsigned int zoomLoc = 0;
	unsigned int color1Loc = 0;
	unsigned int color2Loc = 0;
	unsigned int color3Loc = 0;
	unsigned int color4Loc = 0;
	unsigned int iterationsLoc = 0;

	// functions
	template<typename T>
	static inline T mapToReal(int x, int width, T minR, T maxR);

	template<typename T>
	static inline T mapToImaginary(int y, int height, T minI, T maxI);
	
public:
	Application();

	void Run();

	static std::unique_ptr<Application>& GetInstance();
};

