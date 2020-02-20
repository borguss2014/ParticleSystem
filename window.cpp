#ifdef __APPLE__
    #define GL_SILENCE_DEPRECATION
#endif

#include "window.h"

#include <iostream>
#include <GLFW/glfw3.h>

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebuffer_callback(GLFWwindow* window, int width, int height);

void window::Init(window_props props)
{
#ifdef __APPLE__
    int screenWidth = 1920;
    int screenHeight = 1080;
#else
	int screenWidth = 3840;
	int screenHeight = 2160;
#endif
    
    windowProperties = props;
    
	if (!glfwInit()) {
		std::cout << "Glfw not initialized" << std::endl;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

	m_Window = glfwCreateWindow(props.width, props.height, props.title.c_str(), nullptr, nullptr);
	if (m_Window == nullptr)
	{
		std::cout << "Window couldn't be created" << std::endl;
		glfwTerminate();
	}

	glfwMakeContextCurrent(m_Window);

	glfwSwapInterval(0);
	glfwSetKeyCallback(m_Window, key_callback);
	glfwSetFramebufferSizeCallback(m_Window, framebuffer_callback);
	glfwSetWindowPos(m_Window, screenWidth / 2 - props.width / 2, screenHeight / 2 - props.height / 2);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}

void framebuffer_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}
