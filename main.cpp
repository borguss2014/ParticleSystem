#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <stdio.h>

#define GLFW_INCLUDE_NONE

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
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    const char* glsl_version = "#version 410";
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window.m_Window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    bool openWindow = true;
    
//    particle_data data;
//    data.position = glm::vec2(0.0f, 0.0f);
//    data.velocity = glm::vec2(1.0f, 1.0f);
//    data.colorBegin = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
//    data.colorEnd = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
//    data.scaleBegin = glm::vec3(0.0f, 0.0f, 0.0f);
//    data.scaleEnd = glm::vec3(4.5f, 4.5f, 0.0f);
//    data.totalLife = 3;
//    data.emissionRate = 10;
//    data.emissionFrequency = 1.0f;
   
    particle_system particleSystem;
    particleSystem.Init();
    
//    particleSystem.SetAttribute(particle_attribute::POSITION, glm::vec2(0.0f, 0.0f));
//    particleSystem.SetAttribute(particle_attribute::VELOCITY, glm::vec2(1.0f, 1.0f));
//    particleSystem.SetAttribute(particle_attribute::COLOR_BEGIN, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
//    particleSystem.SetAttribute(particle_attribute::COLOR_END, glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
//    particleSystem.SetAttribute(particle_attribute::SCALE_BEGIN, glm::vec3(0.0f, 0.0f, 0.0f));
//    particleSystem.SetAttribute(particle_attribute::SCALE_END, glm::vec3(4.5f, 4.5f, 0.0f));
//    particleSystem.SetAttribute(particle_attribute::TOTAL_LIFE, 3);
//    particleSystem.SetAttribute(particle_attribute::EMISSION_RATE, 10);
//    particleSystem.SetAttribute(particle_attribute::EMISSION_FREQUENCY, 1.0f);
    
    particleSystem.looping = true;
    particleSystem.Emit();
    
    glm::vec4 myColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	while (!glfwWindowShouldClose(window.m_Window))
	{
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Another Window", &openWindow);
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me")) {
            glfwSetWindowShouldClose(window.m_Window, true);
        }
        ImGui::ColorEdit4("Color", (float*)&myColor);
        ImGui::End();

        ImGui::Render();

        float currentTime = glfwGetTime();
        float delta = currentTime - lastTime;
        lastTime = currentTime;
        
        timestep ts = delta;
        
        std::stringstream ss;
        ss << window.windowProperties.title << " " << ts.GetMilliseconds() << " ms";

        glfwSetWindowTitle(window.m_Window, ss.str().c_str());
        
        glClearColor(myColor.r, myColor.g, myColor.b, myColor.a);
		glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        particleSystem.Update(ts);

		glfwPollEvents();
		glfwSwapBuffers(window.m_Window);
	}

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window.m_Window);
	glfwTerminate();
	return 0;
}
