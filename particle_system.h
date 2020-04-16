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
	glm::vec4 colorBegin;
	glm::vec4 colorEnd;
	glm::vec2 scaleBegin;
	glm::vec2 scaleEnd;
	glm::vec2 speed;
    int emitQuantity; // Particles per time period
	float emissionFrequency;
	float totalLife;
	float currentLife;
};

enum particle_attribute
{
	POSITION			= 0x01,
	SPEED				= 0x02,
	COLOR_BEGIN			= 0x04,
	COLOR_END			= 0x08,
	SCALE_BEGIN			= 0x10,
	SCALE_END			= 0x20,
	EMISSION_RATE		= 0x40,
	EMISSION_FREQUENCY	= 0x80,
	TOTAL_LIFE			= 0x100
};

struct random_distributions
{
	glm::vec2 posXRange	  = glm::vec2(-80, 80);
	glm::vec2 posYRange   = glm::vec2(-80, 80);
	glm::vec2 speedXRange = glm::vec2(1, 5);
	glm::vec2 speedYRange = glm::vec2(1, 5);
	glm::vec2 lifeRange   = glm::vec2(0.1, 10);
	/*glm::vec2 scaleXRange = glm::vec2(0);
	glm::vec2 scaleYRange = glm::vec2(4.5, 4.5);
	glm::vec4 firstColor  = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
	glm::vec4 lastColor   = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);*/
};

struct particle_system
{
	particle_system(int maxParticles = 10000);

    // PARTICLE PROPERTIES
	int totalParticles;
	int lastActiveParticle = -1;
    
    double msElapsed = 0;
    bool emitting = false;
    bool looping = true;
	unsigned short int randomOptions = 0x00;

	// PARTICLES
	std::unique_ptr<std::vector<glm::vec2>> position;
	std::unique_ptr<std::vector<glm::vec2>> speed;
	std::unique_ptr<std::vector<glm::vec4>> colorBegin;
	std::unique_ptr<std::vector<glm::vec4>> colorEnd;
	std::unique_ptr<std::vector<glm::vec4>> color;
	std::unique_ptr<std::vector<glm::vec3>> scaleBegin;
	std::unique_ptr<std::vector<glm::vec3>> scaleEnd;
	std::unique_ptr<std::vector<glm::vec3>> scale;
	std::unique_ptr<std::vector<float>> currentLife;
	std::unique_ptr<std::vector<float>> totalLife;
	std::unique_ptr<std::vector<glm::mat4>> models;
    
    particle_data particleData;
	random_distributions rDistr;

	void Init();
	void Emit();
	void CreateParticle(const particle_data& data);
	void Update(timestep ts);
	void SwapData(const int a, const int b);
	void Destroy(const int index);
	void Stop();

	inline int GetActiveParticles() { return(lastActiveParticle + 1); };
	
	void ParticleBurst(unsigned int nrParticles);
	void ClearParticles();

	void SetRandom(const particle_attribute attribute, bool value);
	void RandomizeParticleAttributes();

	void UploadToGPU();
	void Render();

	GLuint VAO, VBO, EBO, MODELS_VBO, COLORS_VBO;
    std::unique_ptr<Shader> particlesShader;
};