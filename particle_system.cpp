#include "particle_system.h"

#include "window.h"
#include <random>

#define VERTEX_COMPONENTS 2
#define COLOR_COMPONENTS  4
#define VERTICES_PER_QUAD 4
#define INDICES_PER_QUAD  6

std::default_random_engine generator;
std::uniform_real_distribution<double> distribution(-10.0, 10.0);

particle_system::particle_system(int maxParticles)
	: totalParticles(maxParticles)
{    
	position    = std::make_unique<std::vector<glm::vec2>>();
	speed       = std::make_unique<std::vector<glm::vec2>>();
	colorBegin  = std::make_unique<std::vector<glm::vec4>>();
	colorEnd    = std::make_unique<std::vector<glm::vec4>>();
	color       = std::make_unique<std::vector<glm::vec4>>();
    scaleBegin  = std::make_unique<std::vector<glm::vec3>>();
    scaleEnd    = std::make_unique<std::vector<glm::vec3>>();
    scale       = std::make_unique<std::vector<glm::vec3>>();
	currentLife = std::make_unique<std::vector<float>>();
    totalLife   = std::make_unique<std::vector<float>>();
    models      = std::make_unique<std::vector<glm::mat4>>();

	position    ->resize(totalParticles);
	speed       ->resize(totalParticles);
	colorBegin  ->resize(totalParticles);
	colorEnd    ->resize(totalParticles);
	color       ->resize(totalParticles);
    scaleBegin  ->resize(totalParticles);
    scaleEnd    ->resize(totalParticles);
    scale       ->resize(totalParticles);
    currentLife ->resize(totalParticles);
	totalLife   ->resize(totalParticles);
    models      ->resize(totalParticles);
}

void particle_system::Init()
{
    int vertexAttribIndex = 0;
    int colorAttribIndex  = 1;

    float pVerts[] = {
        0.5f,  0.5f,
        0.5f, -0.5f,
       -0.5f, -0.5f,
       -0.5f,  0.5f,
    };

    unsigned int pIndices[] = {
        0, 1, 3,
        1, 2, 3
    };

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
    glGenBuffers(1, &MODELS_VBO);
    glGenBuffers(1, &COLORS_VBO);

	glBindVertexArray(VAO);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pIndices), pIndices, GL_STATIC_DRAW);
    
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pVerts), pVerts, GL_STATIC_DRAW);

    glEnableVertexAttribArray(vertexAttribIndex);
    glVertexAttribPointer(vertexAttribIndex,
        VERTEX_COMPONENTS,
        GL_FLOAT,
        GL_FALSE,
        VERTEX_COMPONENTS * sizeof(GLfloat),
        (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, COLORS_VBO);
    glBufferData(GL_ARRAY_BUFFER,  totalParticles * sizeof(glm::vec4), nullptr, GL_STREAM_DRAW);
    glEnableVertexAttribArray(colorAttribIndex);
    glVertexAttribPointer(colorAttribIndex,
        COLOR_COMPONENTS,
        GL_FLOAT,
        GL_FALSE,
        COLOR_COMPONENTS * sizeof(GLfloat),
        (void*)0);
    glVertexAttribDivisor(colorAttribIndex, 1);

    glBindBuffer(GL_ARRAY_BUFFER, MODELS_VBO);
    glBufferData(GL_ARRAY_BUFFER, totalParticles * sizeof(glm::mat4), nullptr, GL_STREAM_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);

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
        CreateParticle(particleData);
        std::cout << "Particle @ index " << lastActiveParticle << " created" << std::endl;
    }
    emitting = true;
}

void particle_system::CreateParticle(const particle_data& data)
{
    int firstInactivePIndex = lastActiveParticle + 1;
    (*position)[firstInactivePIndex]    = data.position;
    (*speed)[firstInactivePIndex] = 
        glm::vec2(distribution(generator) * data.speed.x, distribution(generator) * data.speed.y);
    (*colorBegin)[firstInactivePIndex]  = data.colorBegin;
    (*colorEnd)[firstInactivePIndex]    = data.colorEnd;
    (*scaleBegin)[firstInactivePIndex]  = glm::vec3(data.scaleBegin, 0.0f);
    (*scaleEnd)[firstInactivePIndex]    = glm::vec3(data.scaleEnd, 0.0f);
    (*currentLife)[firstInactivePIndex] = data.totalLife;
    (*totalLife)[firstInactivePIndex]   = data.totalLife;

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(data.position, 0.0f));
    model = glm::scale(model, glm::vec3(data.scaleEnd, 0.0f));

    (*models)[firstInactivePIndex] = model;

    lastActiveParticle++;
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
        (*position)[i]    += (*speed)[i] * delta;

        // Lerp begin & end colors based on remaining life
        (*color)[i] = glm::mix((*colorEnd)[i], (*colorBegin)[i], (*currentLife)[i] / (*totalLife)[i]);
        (*scale)[i] = glm::mix((*scaleEnd)[i], (*scaleBegin)[i], (*currentLife)[i] / (*totalLife)[i]);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3((*position)[i], 0.0f));
        model = glm::scale(model, (*scale)[i]);

        (*models)[i] = model;
    }

    //printf("State updated \n");

    // Timed particle emission
    if (emitting) {
        if (lastActiveParticle + 1 < totalParticles) {
            if (msElapsed >= ((particleData.emissionFrequency * 1000) / particleData.emitQuantity) && looping) {

                if (!(randomOptions & 0x00)) {
                    RandomizeParticleAttributes();
                }

                CreateParticle(particleData);
                msElapsed = 0;
                // std::cout << "Particle @ index " << lastActiveParticle << " created" << std::endl;
            }
        }
        else {
            //std::cout << "Limit reached! Cannot add more particles" << std::endl;
        }
    }

    UploadToGPU();
    Render();
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

void particle_system::Stop()
{
    emitting = false;
}

void particle_system::ParticleBurst(unsigned int nrParticles)
{
    for (int i = 0; i < nrParticles; i++) {
        CreateParticle(particleData);
    }
}

void particle_system::ClearParticles()
{
    lastActiveParticle = -1;
}

void particle_system::SwapData(const int a, const int b)
{
	std::swap((*position)[a],    (*position)[b]);
	std::swap((*speed)[a],       (*speed)[b]);
	std::swap((*colorBegin)[a],  (*colorBegin)[b]);
	std::swap((*colorEnd)[a],    (*colorEnd)[b]);
	std::swap((*color)[a],       (*color)[b]);
    std::swap((*scaleBegin)[a],  (*scaleBegin)[b]);
    std::swap((*scaleEnd)[a],    (*scaleEnd)[b]);
    std::swap((*scale)[a],       (*scale)[b]);
    std::swap((*currentLife)[a], (*currentLife)[b]);
	std::swap((*totalLife)[a],   (*totalLife)[b]);
    std::swap((*models)[a],      (*models)[b]);
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
        std::uniform_real_distribution<double> pos_randX(rDistr.posXRange.x, rDistr.posXRange.y);
        std::uniform_real_distribution<double> pos_randY(rDistr.posYRange.x, rDistr.posYRange.y);
        particleData.position.x = pos_randX(generator);
        particleData.position.y = pos_randY(generator);
    }

    if (randomOptions & SPEED) {
        std::uniform_real_distribution<double> speed_randX(rDistr.speedXRange.x, rDistr.speedXRange.y);
        std::uniform_real_distribution<double> speed_randY(rDistr.speedYRange.x, rDistr.speedYRange.y);
        particleData.speed.x = speed_randX(generator);
        particleData.speed.y = speed_randY(generator);
    }

    if (randomOptions & TOTAL_LIFE) {
        std::uniform_real_distribution<double> particleLifeRand(rDistr.lifeRange.x, rDistr.lifeRange.y);
        particleData.totalLife = particleLifeRand(generator);
    }

   /* if (randomOptions & SCALE_BEGIN) {

    }

    if (randomOptions & SCALE_END) {

    }

    if (randomOptions & COLOR_BEGIN) {

    }

    if (randomOptions & COLOR_END) {

    }*/
}

void particle_system::UploadToGPU()
{
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, COLORS_VBO);
    glBufferSubData(GL_ARRAY_BUFFER,
        0,
        color->size() * sizeof(glm::vec4),
        &(*color)[0]);
    glBindBuffer(GL_ARRAY_BUFFER, MODELS_VBO);
    glBufferSubData(GL_ARRAY_BUFFER,
        0,
        models->size() * sizeof(glm::mat4),
        &(*models)[0]);
}

void particle_system::Render()
{
    particlesShader->Bind();

    glm::mat4 view       = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    float wWidth  = window::s_Instance->windowProperties.width;
    float wHeight = window::s_Instance->windowProperties.height;

    view        = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.0f));
    projection  = glm::ortho(0.0f, wWidth, wHeight, 0.0f);

    int viewLoc  = glGetUniformLocation(particlesShader->ID, "view");
    int projLoc  = glGetUniformLocation(particlesShader->ID, "projection");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(VAO);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, totalParticles);
    //glDrawArraysInstanced(GL_TRIANGLES, 0, 4, totalParticles);
    glBindVertexArray(0);
}