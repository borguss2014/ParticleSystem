#ifdef __APPLE__
    #define GL_SILENCE_DEPRECATION
#endif

#include "window.h"

#include <iostream>
#include <GLFW/glfw3.h>

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebuffer_callback(GLFWwindow* window, int width, int height);
void cursor_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_buttons_callback(GLFWwindow* window, int button, int action, int mods);

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

	if (s_Instance == nullptr) {
		s_Instance = this;
	}
    
	if (!glfwInit()) {
		std::cout << "Glfw not initialized" << std::endl;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

	m_Window = glfwCreateWindow(props.width, props.height, props.title.c_str(), nullptr, nullptr);
	if (m_Window == nullptr) {
		std::cout << "Window couldn't be created" << std::endl;
		glfwTerminate();
	}

	glfwSetWindowUserPointer(m_Window, (void*)&mouseState);
	glfwMakeContextCurrent(m_Window);

	glViewport(0, 0, props.width, props.height);
	glfwSwapInterval(0);
	glfwSetKeyCallback(m_Window, key_callback);
	glfwSetCursorPosCallback(m_Window, cursor_callback);
	glfwSetMouseButtonCallback(m_Window, mouse_buttons_callback);
	glfwSetFramebufferSizeCallback(m_Window, framebuffer_callback);
	glfwSetWindowPos(m_Window, screenWidth / 2 - props.width / 2, screenHeight / 2 - props.height / 2);
	glfwSetWindowTitle(m_Window, windowProperties.title.c_str());

	glfwGetCursorPos(m_Window, &mouseState.xPos, &mouseState.yPos);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}

void cursor_callback(GLFWwindow* window, double xpos, double ypos)
{
	mouse_state* state = (mouse_state*)glfwGetWindowUserPointer(window);
	state->xPos = xpos;
	state->yPos = ypos;
}

void mouse_buttons_callback(GLFWwindow* window, int button, int action, int mods)
{
	mouse_state* state = (mouse_state*)glfwGetWindowUserPointer(window);

	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			state->leftButtonClicked = true;
		}
		else if (action == GLFW_RELEASE) {
			state->leftButtonClicked = false;
		}
	}
}

void framebuffer_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}