#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include "timestep.h"
#include "Shader.h"

struct particle_data
{
	glm::vec2 position;
	glm::vec2 velocity;
	glm::vec4 colorBegin;
	glm::vec4 colorEnd;
	glm::vec3 scaleBegin;
	glm::vec3 scaleEnd;
    int emissionRate; // Particles per second
	float emissionFrequency;
	float totalLife;
};

struct particle_system
{
	particle_system(const particle_data& pAttributes, int maxParticles = 1000);

    // PARTICLE PROPERTIES
    int particlesToEmit = 0;
	int totalParticles;
	int lastActiveParticle = -1;

	int vertexComponents = 3;
	int colorComponents = 4;
	int scaleComponents = 3;
	int vertsPerQuad = 4;
    int indicesPerQuad = 6;
    
    double msElapsed = 0;
    bool emitting = false;
    bool looping = true;

	// PARTICLES
	std::unique_ptr<std::vector<glm::vec2>> position;
	std::unique_ptr<std::vector<glm::vec2>> velocity;
	std::unique_ptr<std::vector<glm::vec4>> colorBegin;
	std::unique_ptr<std::vector<glm::vec4>> colorEnd;
	std::unique_ptr<std::vector<glm::vec3>> scaleBegin;
	std::unique_ptr<std::vector<glm::vec3>> scaleEnd;
	std::unique_ptr<std::vector<glm::vec4>> color;
	std::unique_ptr<std::vector<glm::vec3>> scale;
	std::unique_ptr<std::vector<float>> totalLife;

	std::unique_ptr<std::vector<float>> compiledData;
    std::unique_ptr<std::vector<int>> compiledDataIndex;
    
    particle_data particleAttr;

	void Init();
	void Emit();
	void Update(timestep ts);
    void Stop();
	void Destroy(const int index);
	void SwapData(const int a, const int b);

	void PrepareUploadData();
	void UploadToGPU();
	void Render();

	GLuint VAO, VBO, EBO;
    std::unique_ptr<Shader> particlesShader;
};
