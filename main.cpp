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
    data.colorEnd = glm::vec4(0.0f, 0.1f, 0.25f, 0.0f);
    data.totalLife = 8;
    data.emissionRate = 100;
   
    particle_system particleSystem(data);
    particleSystem.Init();
    particleSystem.looping = true;
    particleSystem.Emit();
    
//    GLint indices[] = {
//        0, 1, 3,   // first triangle
//        1, 2, 3,    // second triangle
//
//        4, 5, 7,
//        5, 6, 7,
//
//        8, 9, 11,
//        9, 10, 11
//    };
//
//    GLfloat vertices[] = {
//        0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
//         0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
//        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
//        -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
//
//        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
//        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
//        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
//        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
//
//        0.8f,  0.3f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
//         0.8f, -0.3f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
//        -0.8f, -0.3f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
//        -0.8f,  0.3f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
//    };
//
//    std::unique_ptr<std::vector<float>> posData = std::make_unique<std::vector<float>>();
//    posData->resize(84);
//    posData->assign(std::begin(vertices), std::end(vertices));
//
//    for(int i=0; i<84; i++) {
//        std::cout << posData->at(i) << " ";
//
//        if( (i+1) % 7 == 0 ) {
//            std::cout << std::endl;
//        }
//    }
//
//    GLuint VAO,VBO,EBO;
//
//    glGenVertexArrays(1, &VAO);
//    glGenBuffers(1, &VBO);
//    glGenBuffers(1, &EBO);
//
//    glBindVertexArray(VAO);
//
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices, GL_STATIC_DRAW);
//
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    glBufferData(GL_ARRAY_BUFFER, posData->size() * sizeof(GLfloat), &posData->at(0), GL_STATIC_DRAW);
//
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);
//    glEnableVertexAttribArray(0);
//
//    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
//    glEnableVertexAttribArray(1);
//
//    Shader particlesShader("Shaders/vertex.vs", "Shaders/fragment.fs");
//
//    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_BLEND);

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
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
//        particlesShader.Bind();
//        glBindVertexArray(VAO);
//        glDrawElements(GL_TRIANGLES, sizeof(vertices) * sizeof(GLfloat), GL_UNSIGNED_INT, 0);
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        particleSystem.Update(ts);

		glfwPollEvents();
		glfwSwapBuffers(window.m_Window);
	}

	glfwTerminate();
	return 0;
}
