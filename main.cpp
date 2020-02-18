#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "window.h"
#include "Shader.h"
#include "particle_system.h"

float lastTime = 0;

int main(int argc, char* argv[])
{
	window_props windowProps;
#ifdef __APPLE__
	windowProps.width = 800;
	windowProps.height = 600;
#else
    windowProps.width = 1920;
    windowProps.height = 1080;
#endif
	windowProps.title = "Particle system";

	window window;
	window.Init(windowProps);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    particle_data data;
    data.position = glm::vec2(0.0f, 0.0f);
    data.velocity = glm::vec2(0.1f, 0.1f);
    data.colorBegin = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    data.colorEnd = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    data.totalLife = 4.0f;
    data.emissionRate = 5;
   
    particle_system particleSystem(data);
    
    particleSystem.Init();
    particleSystem.Emit();

	while (!glfwWindowShouldClose(window.m_Window))
	{
        float currentTime = glfwGetTime();
        float delta = currentTime - lastTime;
        lastTime = currentTime;
        
        timestep ts = delta;
        
        std::stringstream ss;
        ss << window.windowProperties.title << " " << ts.GetMilliseconds() << " ms";

        glfwSetWindowTitle(window.m_Window, ss.str().c_str());
        
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
        
        particleSystem.Update(ts);

		glfwPollEvents();
		glfwSwapBuffers(window.m_Window);
	}

	glfwTerminate();
	return 0;
}
