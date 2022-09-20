#pragma once
#include <GLFW/glfw3.h>

class Window
{
private:
    static GLFWwindow* m_Window;

public:

    static void Init(const char* title, int width, int height);

    inline static GLFWwindow*& GetWindow() { return m_Window; };
};