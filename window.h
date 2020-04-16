#pragma once

#include <string>

struct GLFWwindow;

struct window_props
{
	int  width = 0;
	int	height = 0;
	std::string title;
};

struct mouse_state
{
	double xPos = 0;
	double yPos = 0;
	bool leftButtonClicked = false;
};

struct window
{
	void Init(window_props props);

	GLFWwindow*		     m_Window;
    window_props windowProperties;
	mouse_state		   mouseState;

	static window* s_Instance;
};