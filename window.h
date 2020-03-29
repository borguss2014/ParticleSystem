#pragma once

#include <string>

struct GLFWwindow;

struct window_props
{
	int			width;
	int			height;
	std::string title;
};

struct window
{
	void Init(window_props props);

	GLFWwindow* m_Window;
    window_props windowProperties;

	static window* s_Instance;
};