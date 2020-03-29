#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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
window* window::s_Instance = nullptr;

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
    bool pSystemEmit = true;
    bool randomPos = false;
    bool randomSpeed = false;
    bool randomParticleLife = false;
   
    particle_data data = {};
    data.position = glm::vec2(0.0f, 0.0f);
    data.speedX = 1;
    data.speedY = 1;
    data.colorBegin = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    data.colorEnd = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
    data.scaleBegin = glm::vec3(0.0f, 0.0f, 0.0f);
    data.scaleEnd = glm::vec3(4.5f, 4.5f, 0.0f);
    data.totalLife = 3;
    data.emitQuantity = 100;
    data.emissionFrequency = 10.0f;

    particle_system particleSystem;
    particleSystem.particleData = data;
    particleSystem.looping = true;

    particleSystem.Init();
    particleSystem.Emit();
    
    glm::vec4 myColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    ImVec2 pos = ImVec2(0.0f, 0.0f);
    ImVec2 size = ImVec2(1000.0f, 1000.0f);

    float elapsedTime = 0.0f;
    int fps = 0;
    int totalFps = 0;

	while (!glfwWindowShouldClose(window.m_Window))
	{
        glClearColor(myColor.r, myColor.g, myColor.b, myColor.a);
        glClear(GL_COLOR_BUFFER_BIT);

        float currentTime = glfwGetTime();
        float delta = currentTime - lastTime;
        lastTime = currentTime;
        
        timestep ts = delta;
        elapsedTime += ts.GetMilliseconds();
        fps++;

        if (elapsedTime >= 1000) {
            totalFps = fps;
            fps = 0;
            elapsedTime = 0;
        }
        
        std::stringstream ss;
        ss << "FPS: " << totalFps << " | " << "Ms / frame: " << floorf(ts.GetMilliseconds() * 100) / 100;

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // RENDERER INFO WINDOW
        {
            ImGui::Begin("Render device");
            ImVec4 textColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
            ImGui::TextColored(textColor, "Vendor: "); ImGui::SameLine();
            ImGui::Text((char*)glGetString(GL_VENDOR));
            ImGui::TextColored(textColor, "Renderer: "); ImGui::SameLine();
            ImGui::Text((char*)glGetString(GL_RENDERER));
            ImGui::TextColored(textColor, "OpenGL version: "); ImGui::SameLine();
            ImGui::Text((char*)glGetString(GL_VERSION));
            ImGui::TextColored(textColor, "GLSL version: "); ImGui::SameLine();
            ImGui::Text((char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
            ImGui::Separator();
            ImGui::TextColored(textColor, "Render info: "); ImGui::SameLine();
            ImGui::Text(ss.str().c_str());
            ImGui::End();
        }

        // PARTICLE SYSTEM WINDOW
        {
            ImGui::Begin("Particle system", &openWindow);

            ImGui::SetWindowPos(pos, ImGuiCond_FirstUseEver);
            ImGui::SetWindowSize(size, ImGuiCond_FirstUseEver);

            bool checkboxTicked = ImGui::Checkbox("Continuous Emission", &pSystemEmit);
            if (checkboxTicked) {
                if (pSystemEmit) {
                    particleSystem.Emit();
                }
                else {
                    particleSystem.Stop();
                }
            }

            // Randomize section
            {
                ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), "Randomize"); 

                ImGui::SameLine();
                checkboxTicked = ImGui::Checkbox("Emission origin", &randomPos);
                if (checkboxTicked) {
                    if (randomPos) {
                        particleSystem.SetRandom(particle_attribute::POSITION, true);
                    }
                    else {
                        particleSystem.SetRandom(particle_attribute::POSITION, false);
                        particleSystem.particleData.position = glm::vec2(0.0f, 0.0f);
                    }
                }

                ImGui::SameLine();
                checkboxTicked = ImGui::Checkbox("Speed", &randomSpeed);
                if (checkboxTicked) {
                    if (randomSpeed) {
                        particleSystem.SetRandom(particle_attribute::SPEED, true);
                    }
                    else {
                        particleSystem.SetRandom(particle_attribute::SPEED, false);
                        particleSystem.particleData.speedX = 1;
                        particleSystem.particleData.speedY = 1;
                    }
                }

                ImGui::SameLine();
                checkboxTicked = ImGui::Checkbox("Particle life", &randomParticleLife);
                if (checkboxTicked) {
                    if (randomParticleLife) {
                        particleSystem.SetRandom(particle_attribute::TOTAL_LIFE, true);
                    }
                    else {
                        particleSystem.SetRandom(particle_attribute::TOTAL_LIFE, false);
                        particleSystem.particleData.totalLife = 3;
                    }
                }
            }

            // System control
            {
                ImGui::TextColored(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), "System control");
                ImGui::InputFloat2("Position", (float*)&particleSystem.particleData.position);
                ImGui::ColorEdit4("CLEAR COLOR", (float*)&myColor);

                if (ImGui::Button("Close")) {
                    glfwSetWindowShouldClose(window.m_Window, true);
                }
            }

            ImGui::End();
        }
        ImGui::Render();

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