#include "particle_system.h"

#include "window.h"
#include <random>

#define VERTEX_COMPONENTS 3
#define COLOR_COMPONENTS  4
#define VERTICES_PER_QUAD 4
#define INDICES_PER_QUAD  6

std::default_random_engine generator;
std::uniform_real_distribution<double> distribution(-10.0, 10.0);

particle_system::particle_system(int maxParticles)
	: totalParticles(maxParticles)
{    
	position    = std::make_unique<std::vector<glm::vec2>>();
	velocity    = std::make_unique<std::vector<glm::vec2>>();
	colorBegin  = std::make_unique<std::vector<glm::vec4>>();
	colorEnd    = std::make_unique<std::vector<glm::vec4>>();
	color       = std::make_unique<std::vector<glm::vec4>>();
    scaleBegin  = std::make_unique<std::vector<glm::vec3>>();
    scaleEnd    = std::make_unique<std::vector<glm::vec3>>();
    scale       = std::make_unique<std::vector<glm::vec3>>();
	currentLife = std::make_unique<std::vector<float>>();
    totalLife   = std::make_unique<std::vector<float>>();

	position    ->resize(totalParticles);
	velocity    ->resize(totalParticles);
	colorBegin  ->resize(totalParticles);
	colorEnd    ->resize(totalParticles);
	color       ->resize(totalParticles);
    scaleBegin  ->resize(totalParticles);
    scaleEnd    ->resize(totalParticles);
    scale       ->resize(totalParticles);
    currentLife ->resize(totalParticles);
	totalLife   ->resize(totalParticles);
    
    int particleSize  = (VERTEX_COMPONENTS + COLOR_COMPONENTS) * VERTICES_PER_QUAD;
    int totalDataSize = particleSize * totalParticles;

	compiledData        = std::make_unique<std::vector<float>>();
    compiledDataIndex   = std::make_unique<std::vector<int>>();
    compiledData        ->resize(totalDataSize);
    compiledDataIndex   ->resize(INDICES_PER_QUAD * totalParticles);
}

void particle_system::Init()
{
    int vertexAttribIndex = 0;
    int colorAttribIndex  = 1;
    
    int pointAttr     = VERTEX_COMPONENTS + COLOR_COMPONENTS;
	int totalDataSize = INDICES_PER_QUAD * totalParticles;
    
    int quadIndex = 0;
    for(int i = 0; i < totalDataSize; i+=6) {
        (*compiledDataIndex)[i]   = 0 + VERTICES_PER_QUAD * quadIndex;
        (*compiledDataIndex)[i+1] = 1 + VERTICES_PER_QUAD * quadIndex;
        (*compiledDataIndex)[i+2] = 3 + VERTICES_PER_QUAD * quadIndex;
        
        (*compiledDataIndex)[i+3] = 1 + VERTICES_PER_QUAD * quadIndex;
        (*compiledDataIndex)[i+4] = 2 + VERTICES_PER_QUAD * quadIndex;
        (*compiledDataIndex)[i+5] = 3 + VERTICES_PER_QUAD * quadIndex;
        quadIndex++;
    }

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                compiledDataIndex->size() * sizeof(GLfloat),
                &(*compiledDataIndex)[0],
                GL_STATIC_DRAW);
    
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER,
                compiledData->size() * sizeof(GLfloat),
                nullptr,
                GL_STREAM_DRAW);

	glVertexAttribPointer(vertexAttribIndex,
                            VERTEX_COMPONENTS,
                            GL_FLOAT,
                            GL_FALSE, 
                            pointAttr * sizeof(GLfloat),
                            (void*)0);
	glEnableVertexAttribArray(vertexAttribIndex);

	glVertexAttribPointer(colorAttribIndex,
                            COLOR_COMPONENTS,
                            GL_FLOAT,
                            GL_FALSE,
                            pointAttr * sizeof(GLfloat),
                            (void*)(VERTEX_COMPONENTS * sizeof(GLfloat)));
	glEnableVertexAttribArray(colorAttribIndex);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    particlesShader = std::make_unique<Shader>("Shaders/vertex.glsl", "Shaders/fragment.glsl");
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	std::cout << "Particle system initialized" << std::endl;
}

void particle_system::Emit()
{
    if (!looping) {
        int firstInactivePIndex = lastActiveParticle + 1;
        (*position)[firstInactivePIndex]    = particleData.position;
        (*velocity)[firstInactivePIndex]    = glm::vec2(distribution(generator) * particleData.speed.x, distribution(generator) * particleData.speed.y);
        (*colorBegin)[firstInactivePIndex]  = particleData.colorBegin;
        (*colorEnd)[firstInactivePIndex]    = particleData.colorEnd;
        (*scaleBegin)[firstInactivePIndex]  = glm::vec3(particleData.scaleBegin, 0.0f);
        (*scaleEnd)[firstInactivePIndex]    = glm::vec3(particleData.scaleEnd, 0.0f);
        (*currentLife)[firstInactivePIndex] = particleData.totalLife;
        (*totalLife)[firstInactivePIndex]   = particleData.totalLife;
        lastActiveParticle++;
        
        std::cout << "Particle @ index " << lastActiveParticle << " created" << std::endl;
    }
    emitting = true;
}

void particle_system::Update(timestep ts)
{
    float delta = ts.GetSeconds();
    msElapsed += ts.GetMilliseconds();

    // Particle state update
    for (int i = 0; i <= lastActiveParticle; i++) {
        float remainingLife = (*currentLife)[i];
        if (remainingLife <= 0.0f) {
            Destroy(i);
        }

        (*currentLife)[i] -= delta;
        (*position)[i]    += (*velocity)[i] * delta;

        // Lerp begin & end colors based on remaining life
        (*color)[i] = glm::mix((*colorEnd)[i], (*colorBegin)[i], (*currentLife)[i] / (*totalLife)[i]);
        (*scale)[i] = glm::mix((*scaleEnd)[i], (*scaleBegin)[i], (*currentLife)[i] / (*totalLife)[i]);
    }

    // Timed particle emission
    if (emitting) {
        if (lastActiveParticle + 1 < totalParticles) {
            if (msElapsed >= ((particleData.emissionFrequency * 1000) / particleData.emitQuantity) && looping) {

                if (!(randomOptions & 0x00)) {
                    RandomizeParticleAttributes();
                }

                // TODO: Allow randomized initial properties
                size_t firstInactivePIndex = (size_t)lastActiveParticle + 1;
                (*position)[firstInactivePIndex]    = particleData.position;
                (*velocity)[firstInactivePIndex]    = glm::vec2(distribution(generator) * particleData.speed.x, distribution(generator) * particleData.speed.y);
                (*colorBegin)[firstInactivePIndex]  = particleData.colorBegin;
                (*colorEnd)[firstInactivePIndex]    = particleData.colorEnd;
                (*scaleBegin)[firstInactivePIndex]  = glm::vec3(particleData.scaleBegin, 0.0f);
                (*scaleEnd)[firstInactivePIndex]    = glm::vec3(particleData.scaleEnd, 0.0f);
                (*currentLife)[firstInactivePIndex] = particleData.totalLife;
                (*totalLife)[firstInactivePIndex]   = particleData.totalLife;
                lastActiveParticle++;

                msElapsed = 0;
                // std::cout << "Particle @ index " << lastActiveParticle << " created" << std::endl;
            }
        }
        else {
            std::cout << "Limit reached! Cannot add more particles" << std::endl;
        }
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
	std::swap((*position)[a],    (*position)[b]);
	std::swap((*velocity)[a],    (*velocity)[b]);
	std::swap((*colorBegin)[a],  (*colorBegin)[b]);
	std::swap((*colorEnd)[a],    (*colorEnd)[b]);
	std::swap((*color)[a],       (*color)[b]);
    std::swap((*scaleBegin)[a],  (*scaleBegin)[b]);
    std::swap((*scaleEnd)[a],    (*scaleEnd)[b]);
    std::swap((*scale)[a],       (*scale)[b]);
    std::swap((*currentLife)[a], (*currentLife)[b]);
	std::swap((*totalLife)[a],   (*totalLife)[b]);
}

void particle_system::SetRandom(const particle_attribute attribute, bool enabled)
{
    if (enabled) {
        randomOptions = randomOptions | attribute;
    }
    else {
        randomOptions = randomOptions & ~attribute;
    }
}

void particle_system::RandomizeParticleAttributes()
{
    if (randomOptions & POSITION) {
        float wWidth = window::s_Instance->windowProperties.width;
        float wHeight = window::s_Instance->windowProperties.height;

        // TODO: Positions should be based on screen coordinates
        std::uniform_real_distribution<double> pos_randX(-80, 80);
        std::uniform_real_distribution<double> pos_randY(-80, 80);
        particleData.position.x = pos_randX(generator);
        particleData.position.y = pos_randY(generator);
    }

    if (randomOptions & SPEED) {
        std::uniform_real_distribution<double> speed_randX(1, 5);
        std::uniform_real_distribution<double> speed_randY(1, 5);
        particleData.speed.x = speed_randX(generator);
        particleData.speed.y = speed_randY(generator);
    }

    if (randomOptions & TOTAL_LIFE) {
        std::uniform_real_distribution<double> particleLifeRand(0.1, 10);
        particleData.totalLife = particleLifeRand(generator);
    }
}

void particle_system::PrepareUploadData()
{
	if (lastActiveParticle >= 0) {
        int dataIndex = 0;
		for (int i = 0; i <= lastActiveParticle; i++) {
            // TODO: Research dynamic batching
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
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    compiledData->size() * sizeof(GLfloat),
                    &(*compiledData)[0]);
}

void particle_system::Render()
{
    particlesShader->Bind();

    glm::mat4 model      = glm::mat4(1.0f);
    glm::mat4 view       = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    float wWidth  = window::s_Instance->windowProperties.width;
    float wHeight = window::s_Instance->windowProperties.height;

    model       = glm::scale(model, glm::vec3(20.0f, 20.0f, 0.0f));
    view        = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.0f));
    projection  = glm::ortho(-wWidth, wWidth, -wHeight, wHeight);

    int modelLoc = glGetUniformLocation(particlesShader->ID, "model");
    int viewLoc  = glGetUniformLocation(particlesShader->ID, "view");
    int projLoc  = glGetUniformLocation(particlesShader->ID, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawElements(GL_TRIANGLES, INDICES_PER_QUAD * (lastActiveParticle+1), GL_UNSIGNED_INT, 0);
}