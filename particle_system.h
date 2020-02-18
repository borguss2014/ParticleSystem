#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "timestep.h"
#include "Shader.h"

struct particle_data
{
	glm::vec2 position;
	glm::vec2 velocity;
	glm::vec4 colorBegin;
	glm::vec4 colorEnd;
    int emissionRate; // Particles per second
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
	int vertsPerQuad = 4;
    int indicesPerQuad = 6;
    
    double msElapsed = 0;
    bool emitting = false;

	// PARTICLES
	std::unique_ptr<std::vector<glm::vec2>> position;
	std::unique_ptr<std::vector<glm::vec2>> velocity;
	std::unique_ptr<std::vector<glm::vec4>> colorBegin;
	std::unique_ptr<std::vector<glm::vec4>> colorEnd;
	std::unique_ptr<std::vector<glm::vec4>> color;
	std::unique_ptr<std::vector<float>> totalLife;

	std::unique_ptr<std::vector<float>> compiledData;
    std::unique_ptr<std::vector<int>> compiledDataIndex;
    
    particle_data particleAttr;

	void Init();
	void Emit();
    void Stop();
	void Destroy(const int index);
	void SwapData(const int a, const int b);

	void PrepareUploadData();
	void UploadToGPU();

	void Update(timestep ts);
	void Render();

	GLuint VAO, VBO, EBO;
    std::unique_ptr<Shader> particlesShader;
};
