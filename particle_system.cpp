#include "particle_system.h"

particle_system::particle_system(const particle_data& pAttributes, int maxParticles)
	: totalParticles(maxParticles), particleAttr(pAttributes)
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
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(colorAttribIndex, colorComponents, GL_FLOAT, GL_FALSE, pointAttr * sizeof(GLfloat), (void*)(vertexComponents * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    particlesShader = std::make_unique<Shader>("Shaders/vertex.vs", "Shaders/fragment.fs");
    
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH);

	std::cout << "Particle system initialized" << std::endl;
}

void particle_system::Emit()
{
    if (!looping) {
        int firstInactivePIndex = lastActiveParticle + 1;
        (*position)[firstInactivePIndex] = particleAttr.position;
        (*velocity)[firstInactivePIndex] = particleAttr.velocity;
        (*colorBegin)[firstInactivePIndex] = particleAttr.colorBegin;
        (*colorEnd)[firstInactivePIndex] = particleAttr.colorEnd;
        (*totalLife)[firstInactivePIndex] = particleAttr.totalLife;
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
            (*color)[i] = glm::mix((*colorEnd)[i], (*colorBegin)[i], remainingLife / particleAttr.totalLife);
        }
    }

    // Timed particle emission
    if (lastActiveParticle + 1 < totalParticles) {
        if (msElapsed >= (1000 / particleAttr.emissionRate) && looping) {
            size_t firstInactivePIndex = (size_t)lastActiveParticle + 1;
            (*position)[firstInactivePIndex] = particleAttr.position;
            (*velocity)[firstInactivePIndex] = particleAttr.velocity;
            (*colorBegin)[firstInactivePIndex] = particleAttr.colorBegin;
            (*colorEnd)[firstInactivePIndex] = particleAttr.colorEnd;
            (*totalLife)[firstInactivePIndex] = particleAttr.totalLife;
            lastActiveParticle++;

            msElapsed = 0;
            std::cout << "Particle @ index " << lastActiveParticle << " created" << std::endl;
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
        std::cout << "Particle destroy: Swapping particle @ " << index << " with " << lastActiveParticle << std::endl;
    }
    else {
        std::cout << "Destroy last particle" << std::endl;
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
	std::swap((*totalLife)[a], (*totalLife)[b]);
}

void particle_system::PrepareUploadData()
{
	if (lastActiveParticle >= 0) {
        int dataIndex = 0;
		for (int i = 0; i <= lastActiveParticle; i++) {
            (*compiledData)[dataIndex++] = (*position)[i].x + 0.5f;
            (*compiledData)[dataIndex++] = (*position)[i].y + 0.5f;
            (*compiledData)[dataIndex++] = 0.0f;
            (*compiledData)[dataIndex++] = (*color)[i].r;
            (*compiledData)[dataIndex++] = (*color)[i].g;
            (*compiledData)[dataIndex++] = (*color)[i].b;
            (*compiledData)[dataIndex++] = (*color)[i].a;

            (*compiledData)[dataIndex++] = (*position)[i].x + 0.5f;
            (*compiledData)[dataIndex++] = (*position)[i].y - 0.5f;
            (*compiledData)[dataIndex++] = 0.0f;
            (*compiledData)[dataIndex++] = (*color)[i].r;
            (*compiledData)[dataIndex++] = (*color)[i].g;
            (*compiledData)[dataIndex++] = (*color)[i].b;
            (*compiledData)[dataIndex++] = (*color)[i].a;

            (*compiledData)[dataIndex++] = (*position)[i].x - 0.5f;
            (*compiledData)[dataIndex++] = (*position)[i].y - 0.5f;
            (*compiledData)[dataIndex++] = 0.0f;
            (*compiledData)[dataIndex++] = (*color)[i].r;
            (*compiledData)[dataIndex++] = (*color)[i].g;
            (*compiledData)[dataIndex++] = (*color)[i].b;
            (*compiledData)[dataIndex++] = (*color)[i].a;

            (*compiledData)[dataIndex++] = (*position)[i].x - 0.5f;
            (*compiledData)[dataIndex++] = (*position)[i].y + 0.5f;
            (*compiledData)[dataIndex++] = 0.0f;
            (*compiledData)[dataIndex++] = (*color)[i].r;
            (*compiledData)[dataIndex++] = (*color)[i].g;
            (*compiledData)[dataIndex++] = (*color)[i].b;
            (*compiledData)[dataIndex++] = (*color)[i].a;
		}
        

//        for(auto iterator = compiledData->begin(); iterator!=compiledData->end(); std::advance(iterator, 1)) {
//            std::cout << *iterator << " ";
//
//            if(((iterator-compiledData->begin())+1)%7==0) {
//                std::cout << std::endl;
//            }
//        }
//        
//        std::cout << "========================================================================" << std::endl;
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
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    particlesShader->Bind();
    glDrawElements(GL_TRIANGLES, indicesPerQuad * (lastActiveParticle+1), GL_UNSIGNED_INT, 0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}