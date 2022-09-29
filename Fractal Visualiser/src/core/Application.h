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
	static std::unique_ptr<Application> s_Instance;

	// callbacks
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

	// shaders
	Shader m_MandelbrotShader;
	Shader m_BurningshipShader;
	Shader m_TricornShader;
	Shader m_MandelbulbShader;
	Shader* p_SelectedShader = nullptr;

	unsigned int m_ShaderID = 0;

	static GLFWwindow* p_Window;

	// fractal selection
	int p_SelectedFractal = 0;
	static constexpr unsigned int c_NumFractals = 4;
	const char* m_FractalOptions[c_NumFractals] = {"Mandelbrot", "Burning Ship", "Tricorn", "Mandelbulb"};

	// fractal properties - uniforms
	Vec2 m_Location  = {0.0f, 0.0f};
	float m_Zoom = 2.0f;
	bool m_isJuliaMode = false;
	int m_Iterations = 200;

	float m_Color1[3] = {0.5f, 0.5f, 0.5f};
	float m_Color2[3] = { 0.5f, 0.5f, 0.5f };
	float m_Color3[3] = { 1.0f, 1.0f, 1.0f };
	float m_Color4[3] = { 0.0f, 0.33f, 0.67f };

	// other properties - non uniform
	bool m_isJuliaPaused = false;

	float m_MinR = 0.0f;
	float m_MaxR = 0.0f;
	float m_MinI = 0.0f;
	float m_MaxI = 0.0f;
	int m_ScreenWidth = 0;
	int m_ScreenHeight = 0;

	// ui elements
	bool m_shouldRenderGUI = true;

	bool m_isIterationsSliderUsed = false;
	bool m_isJuliaModeCheckboxUsed = false;
	bool m_isFractalSelectorUsed = false;

	bool m_isColor1SelectorUsed = false;
	bool m_isColor2SelectorUsed = false;
	bool m_isColor3SelectorUsed = false;
	bool m_isColor4SelectorUsed = false;

	bool m_isRandomiseColor1ButtonPressed = false;
	bool m_isRandomiseColor2ButtonPressed = false;
	bool m_isRandomiseColor3ButtonPressed = false;
	bool m_isRandomiseColor4ButtonPressed = false;

	bool m_isColorPresetSelectorUsed = false;

	bool m_isRandomiseAllColorsButtonPressed = false;
	bool m_isSavePresetButtonPressed = false;

	// color preset
	int m_SelectedColorPreset = 0;
	static constexpr unsigned int c_NumColorPresets = 2;
	const char* m_ColorPresetOptions[c_NumColorPresets] = { "Preset 1", "Black Pink"};

	float m_ColorPresets[c_NumColorPresets][4][3] = {
		{{0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.33f, 0.67f}}, // Preset 1
		{{0.449f, 0.0f, 0.5f}, {0.5f, 0.173f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.673f, 0.391f, 0.678f}}
	};

	// julia set orbitals
	bool m_isJuliaOrbitOn = false;
	float m_JuliaOrbitSpeed = 1.0f;
	float m_JuliaOrbitRadius = 1.0f;

	// mouse location
	float m_MouseXPos = 0.0f;
	float m_MouseYPos = 0.0f;

	void ProcessInput();
	void CheckUI();

	// uniform locations
	unsigned int m_ResolutionLoc = 0;
	unsigned int m_LocationLoc = 0;
	unsigned int m_MousePosLoc = 0;
	unsigned int m_JuliaModeLoc = 0;
	unsigned int m_ZoomLoc = 0;
	unsigned int m_IterationsLoc = 0;
	unsigned int m_Color1Loc = 0;
	unsigned int m_Color2Loc = 0;
	unsigned int m_Color3Loc = 0;
	unsigned int m_Color4Loc = 0;

	// functions
	template<typename T>
	static inline T LinearInterpolate(int x, int width, T minR, T maxR);

	//utility functiosn for app
	void UpdateShaderMousePosition();
	void UpdateShaderUniformLocations();

	// image saving
	bool save_png_libpng(const char* filename, uint8_t* pixels, int w, int h);

	void RandomiseColor1();
	void RandomiseColor2();
	void RandomiseColor3();
	void RandomiseColor4();


	
public:
	Application();

	void Run();

	static std::unique_ptr<Application>& GetInstance();
};

