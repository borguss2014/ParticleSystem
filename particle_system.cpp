#include "particle_system.h"

particle_system::particle_system(const particle_data& pAttributes, int maxParticles)
	: totalParticles(maxParticles), particleAttributes(pAttributes)
{
	position = std::make_unique<std::vector<glm::vec2>>();
	velocity = std::make_unique<std::vector<glm::vec2>>();
	colorBegin = std::make_unique<std::vector<glm::vec4>>();
	colorEnd = std::make_unique<std::vector<glm::vec4>>();
	color = std::make_unique<std::vector<glm::vec4>>();
	totalLife = std::make_unique<std::vector<float>>();

	position->resize(totalParticles);
	velocity->resize(totalParticles);
	colorBegin->resize(totalParticles);
	colorEnd->resize(totalParticles);
	color->resize(totalParticles);
	totalLife->resize(totalParticles);
    
    int particleSize = vertexComponents * vertsPerQuad + colorComponents * vertsPerQuad;
    int totalDataSize = particleSize * totalParticles;
	compiledData = std::make_unique<std::vector<float>>();
    compiledData->resize(totalDataSize);
}

void particle_system::Init()
{
    int vertexAttribIndex = 0;
    int colorAttribIndex = 1;
    
	int pointComponents = vertexComponents + colorComponents;
	int totalDataSize = 6 * pointComponents * sizeof(GLfloat);

	GLint indices[] = {
		2,3,1,
		3,0,1
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, totalDataSize, nullptr, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), nullptr, GL_STATIC_DRAW);

	glVertexAttribPointer(vertexAttribIndex, vertexComponents, GL_FLOAT, GL_FALSE, pointComponents * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(colorAttribIndex, colorComponents, GL_FLOAT, GL_FALSE, pointComponents * sizeof(GLfloat), (void*)(vertexComponents * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
    
//    particlesShader = std::make_unique<Shader>("Shaders/vertex.vs", "Shaders/fragment.fs");

	std::cout << "Particle system initialized" << std::endl;
}

void particle_system::Emit()
{
	if (lastActiveParticle + 1 == totalParticles) {
		std::cout << "Limit reached! Cannot add more particles" << std::endl;
		return;
	}
    
    emitting = true;
}

void particle_system::Destroy(const int index)
{
    std::cout << "Particle destroy: Swapping particle @ " << index << " with " << lastActiveParticle << std::endl;
    
    if (lastActiveParticle > 0) {
        SwapData(index, lastActiveParticle);
    }
    
    lastActiveParticle--;
}

void particle_system::SwapData(const int a, const int b)
{
	std::swap(position->at(a), position->at(b));
	std::swap(velocity->at(a), velocity->at(b));
	std::swap(colorBegin->at(a), colorBegin->at(b));
	std::swap(colorEnd->at(a), colorEnd->at(b));
	std::swap(color->at(a), color->at(b));
	std::swap(totalLife->at(a), totalLife->at(b));
}

void particle_system::PrepareUploadData()
{
	if (lastActiveParticle >= 0) {
		float quadVertices[] = {
			-0.5f, -0.5f, 0.0f,
			-0.5f,  0.5f, 0.0f,
			 0.5f,  0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f
		};

		int totalPositionFloats = vertexComponents * vertsPerQuad;

		size_t dataIndex = 0;
		for (int i = 0; i <= lastActiveParticle; i++) {
			for (int j = 0; j < totalPositionFloats; j++) {
				compiledData->at(dataIndex) = quadVertices[j];
				dataIndex++;

				if ((j + 1) % 3 == 0) {
					compiledData->at(dataIndex) = color->at(i).r;
					compiledData->at(dataIndex + 1) = color->at(i).g;
					compiledData->at(dataIndex + 2) = color->at(i).b;
					compiledData->at(dataIndex + 3) = color->at(i).a;
					dataIndex += 4;
				}
			}
		} // FOR END
	} // IF END
}

void particle_system::UploadToGPU()
{
    glBindVertexArray(VAO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, compiledData->size() * sizeof(float), &compiledData);
}

void particle_system::Update(timestep ts)
{
	float delta = ts.GetSeconds();
    
    msElapsed += delta;
    
    // Timed particle emission
    if(msElapsed >= emissionRate) {
        size_t firstInactivePIndex = lastActiveParticle + 1;

        position->at(firstInactivePIndex) = particleAttributes.position;
        velocity->at(firstInactivePIndex) = particleAttributes.velocity;
        colorBegin->at(firstInactivePIndex) = particleAttributes.colorBegin;
        colorEnd->at(firstInactivePIndex) = particleAttributes.colorEnd;
        totalLife->at(firstInactivePIndex) = particleAttributes.totalLife;

        lastActiveParticle++;
        msElapsed = 0;
        std::cout << "Particle @ index " << lastActiveParticle << " created" << std::endl;
    }
    
    // Particle state update
	if (lastActiveParticle >= 0) {
		for (int i = 0; i <= lastActiveParticle; i++) {
			float remainingLife = totalLife->at(i);
			if (remainingLife <= 0.0f) {
				Destroy(i);
				continue;
			}

			totalLife->at(i) -= delta;
			position->at(i) += velocity->at(i) * delta;

			// Lerp begin & end colors based on remaining life
			color->at(i) = glm::mix(colorBegin->at(i), colorEnd->at(i), delta);
		}

		PrepareUploadData();
        UploadToGPU();
	}
}

void particle_system::Render()
{
    
}
