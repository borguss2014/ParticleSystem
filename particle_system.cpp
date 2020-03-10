#include "particle_system.h"

#include <random>

std::default_random_engine generator;
std::uniform_real_distribution<double> distribution(-10.0, 10.0);

particle_system::particle_system(int maxParticles)
	: totalParticles(maxParticles)
{
    particle_data initData;
    particleData = initData;
    
	position = std::make_unique<std::vector<glm::vec2>>();
	velocity = std::make_unique<std::vector<glm::vec2>>();
	colorBegin = std::make_unique<std::vector<glm::vec4>>();
	colorEnd = std::make_unique<std::vector<glm::vec4>>();
	color = std::make_unique<std::vector<glm::vec4>>();
    scaleBegin = std::make_unique<std::vector<glm::vec3>>();
    scaleEnd = std::make_unique<std::vector<glm::vec3>>();
    scale = std::make_unique<std::vector<glm::vec3>>();
	totalLife = std::make_unique<std::vector<float>>();

	position->resize(totalParticles);
	velocity->resize(totalParticles);
	colorBegin->resize(totalParticles);
	colorEnd->resize(totalParticles);
	color->resize(totalParticles);
    scaleBegin->resize(totalParticles);
    scaleEnd->resize(totalParticles);
    scale->resize(totalParticles);
	totalLife->resize(totalParticles);
    
    int particleSize = (vertexComponents + colorComponents) * vertsPerQuad;
    int totalDataSize = particleSize * totalParticles;
	compiledData = std::make_unique<std::vector<float>>();
    compiledData->resize(totalDataSize);
    compiledDataIndex = std::make_unique<std::vector<int>>();
    compiledDataIndex->resize(indicesPerQuad * totalParticles);
}

void particle_system::Init()
{
    int vertexAttribIndex = 0;
    int colorAttribIndex = 1;
    
    int pointAttr = vertexComponents + colorComponents;
	int totalDataSize = indicesPerQuad * totalParticles;
    
    int quadIndex = 0;
    for(int i = 0; i < totalDataSize; i+=6) {
        (*compiledDataIndex)[i] = 0 + vertsPerQuad * quadIndex;
        (*compiledDataIndex)[i+1] = 1 + vertsPerQuad * quadIndex;
        (*compiledDataIndex)[i+2] = 3 + vertsPerQuad * quadIndex;
        
        (*compiledDataIndex)[i+3] = 1 + vertsPerQuad * quadIndex;
        (*compiledDataIndex)[i+4] = 2 + vertsPerQuad * quadIndex;
        (*compiledDataIndex)[i+5] = 3 + vertsPerQuad * quadIndex;
        quadIndex++;
    }

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, compiledDataIndex->size() * sizeof(GLfloat), &(*compiledDataIndex)[0], GL_STATIC_DRAW);
    
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, compiledData->size() * sizeof(GLfloat), nullptr, GL_STREAM_DRAW);

	glVertexAttribPointer(vertexAttribIndex, vertexComponents, GL_FLOAT, GL_FALSE, pointAttr * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(vertexAttribIndex);

	glVertexAttribPointer(colorAttribIndex, colorComponents, GL_FLOAT, GL_FALSE, pointAttr * sizeof(GLfloat), (void*)(vertexComponents * sizeof(GLfloat)));
	glEnableVertexAttribArray(colorAttribIndex);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    particlesShader = std::make_unique<Shader>("Shaders/vertex.glsl", "Shaders/fragment.glsl");
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	std::cout << "Particle system initialized" << std::endl;
}

void particle_system::Emit()
{
    if (!looping) {
        int firstInactivePIndex = lastActiveParticle + 1;
        (*position)[firstInactivePIndex] = particleData.position;
        (*velocity)[firstInactivePIndex] = particleData.velocity;
        (*colorBegin)[firstInactivePIndex] = particleData.colorBegin;
        (*colorEnd)[firstInactivePIndex] = particleData.colorEnd;
        (*scaleBegin)[firstInactivePIndex] = particleData.scaleBegin;
        (*scaleEnd)[firstInactivePIndex] = particleData.scaleEnd;
        (*totalLife)[firstInactivePIndex] = particleData.totalLife;
        lastActiveParticle++;
        
        std::cout << "Particle @ index " << lastActiveParticle << " created" << std::endl;
    }
    
    emitting = true;
}

void particle_system::Update(timestep ts)
{
    if (!emitting) {
        return;
    }

    float delta = ts.GetSeconds();
    msElapsed += ts.GetMilliseconds();

    // Particle state update
    if (lastActiveParticle >= 0) {
        for (int i = 0; i <= lastActiveParticle; i++) {
            float remainingLife = (*totalLife)[i];
            if (remainingLife <= 0.0f) {
                Destroy(i);
            }

            (*totalLife)[i] -= delta;
            (*position)[i] += (*velocity)[i] * delta;

            // Lerp begin & end colors based on remaining life
            (*color)[i] = glm::mix((*colorEnd)[i], (*colorBegin)[i], (*totalLife)[i] / particleData.totalLife);
            (*scale)[i] = glm::mix((*scaleBegin)[i], (*scaleEnd)[i], (*totalLife)[i] / particleData.totalLife);
        }
    }

    // Timed particle emission
    if (lastActiveParticle + 1 < totalParticles) {
        if (msElapsed >= ((particleData.emissionFrequency * 1000) / particleData.emissionRate) && looping) {
            // TODO: Allow randomized initial properties
            size_t firstInactivePIndex = (size_t)lastActiveParticle + 1;
            (*position)[firstInactivePIndex] = particleData.position;
            (*velocity)[firstInactivePIndex] = glm::vec2(distribution(generator) * particleData.velocity.x, distribution(generator) * particleData.velocity.y);
            (*colorBegin)[firstInactivePIndex] = particleData.colorBegin;
            (*colorEnd)[firstInactivePIndex] = particleData.colorEnd;
            (*scaleBegin)[firstInactivePIndex] = particleData.scaleBegin;
            (*scaleEnd)[firstInactivePIndex] = particleData.scaleEnd;
            (*totalLife)[firstInactivePIndex] = particleData.totalLife;
            lastActiveParticle++;

            msElapsed = 0;
           // std::cout << "Particle @ index " << lastActiveParticle << " created" << std::endl;
        }
    }
    else {
        std::cout << "Limit reached! Cannot add more particles" << std::endl;
    }

    PrepareUploadData();
    UploadToGPU();
    Render();
}

void particle_system::Stop()
{
    emitting = false;
}

void particle_system::Destroy(const int index)
{
    if (lastActiveParticle > 0) {
        SwapData(index, lastActiveParticle);
        //std::cout << "Particle destroy: Swapping particle @ " << index << " with " << lastActiveParticle << std::endl;
    }
    else {
        //std::cout << "Destroy last particle" << std::endl;
    }
    
    lastActiveParticle--;
}

void particle_system::SwapData(const int a, const int b)
{
	std::swap((*position)[a], (*position)[b]);
	std::swap((*velocity)[a], (*velocity)[b]);
	std::swap((*colorBegin)[a], (*colorBegin)[b]);
	std::swap((*colorEnd)[a], (*colorEnd)[b]);
	std::swap((*color)[a], (*color)[b]);
    std::swap((*scaleBegin)[a], (*scaleBegin)[b]);
    std::swap((*scaleEnd)[a], (*scaleEnd)[b]);
    std::swap((*scale)[a], (*scale)[b]);
	std::swap((*totalLife)[a], (*totalLife)[b]);
}

void particle_system::PrepareUploadData()
{
	if (lastActiveParticle >= 0) {
        int dataIndex = 0;
		for (int i = 0; i <= lastActiveParticle; i++) {
            // TODO: A lot of repeated data in here. Could bundle it better so less data is uploaded each frame
            // Research dynamic batching
            (*compiledData)[dataIndex++] = (*position)[i].x + 0.5f * (*scale)[i].x;
            (*compiledData)[dataIndex++] = (*position)[i].y + 0.5f * (*scale)[i].y;
            (*compiledData)[dataIndex++] = 0.0f;
            (*compiledData)[dataIndex++] = (*color)[i].r;
            (*compiledData)[dataIndex++] = (*color)[i].g;
            (*compiledData)[dataIndex++] = (*color)[i].b;
            (*compiledData)[dataIndex++] = (*color)[i].a;

            (*compiledData)[dataIndex++] = (*position)[i].x + 0.5f * (*scale)[i].x;
            (*compiledData)[dataIndex++] = (*position)[i].y - 0.5f * (*scale)[i].y;
            (*compiledData)[dataIndex++] = 0.0f;
            (*compiledData)[dataIndex++] = (*color)[i].r;
            (*compiledData)[dataIndex++] = (*color)[i].g;
            (*compiledData)[dataIndex++] = (*color)[i].b;
            (*compiledData)[dataIndex++] = (*color)[i].a;

            (*compiledData)[dataIndex++] = (*position)[i].x - 0.5f * (*scale)[i].x;
            (*compiledData)[dataIndex++] = (*position)[i].y - 0.5f * (*scale)[i].y;
            (*compiledData)[dataIndex++] = 0.0f;
            (*compiledData)[dataIndex++] = (*color)[i].r;
            (*compiledData)[dataIndex++] = (*color)[i].g;
            (*compiledData)[dataIndex++] = (*color)[i].b;
            (*compiledData)[dataIndex++] = (*color)[i].a;

            (*compiledData)[dataIndex++] = (*position)[i].x - 0.5f * (*scale)[i].x;
            (*compiledData)[dataIndex++] = (*position)[i].y + 0.5f * (*scale)[i].y;
            (*compiledData)[dataIndex++] = 0.0f;
            (*compiledData)[dataIndex++] = (*color)[i].r;
            (*compiledData)[dataIndex++] = (*color)[i].g;
            (*compiledData)[dataIndex++] = (*color)[i].b;
            (*compiledData)[dataIndex++] = (*color)[i].a;
		}
	}
}

void particle_system::UploadToGPU()
{
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //glBufferData(GL_ARRAY_BUFFER, compiledData->size() * sizeof(GLfloat), nullptr, GL_STREAM_DRAW); //Buffer orphaning
    glBufferSubData(GL_ARRAY_BUFFER, 0, compiledData->size() * sizeof(GLfloat), &(*compiledData)[0]);
}

void particle_system::Render()
{
    particlesShader->Bind();

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    model = glm::scale(model, glm::vec3(20.0f, 20.0f, 0.0f));
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.0f));
    projection = glm::ortho(-1920.0f, 1920.0f, -1080.0f, 1080.0f);

    int modelLoc = glGetUniformLocation(particlesShader->ID, "model");
    int viewLoc = glGetUniformLocation(particlesShader->ID, "view");
    int projLoc = glGetUniformLocation(particlesShader->ID, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawElements(GL_TRIANGLES, indicesPerQuad * (lastActiveParticle+1), GL_UNSIGNED_INT, 0);
}