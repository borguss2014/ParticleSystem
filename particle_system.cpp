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
    
    int particleSize = vertexComponents * vertsPerQuad + colorComponents * vertsPerQuad;
    int totalDataSize = particleSize * totalParticles;
	compiledData = std::make_unique<std::vector<float>>();
    compiledData->resize(totalDataSize);
    compiledDataIndex = std::make_unique<std::vector<int>>();
    compiledDataIndex->resize(indicesPerQuad*totalParticles);
}

void particle_system::Init()
{
    int vertexAttribIndex = 0;
    int colorAttribIndex = 1;
    
	int pointAttr = vertexComponents + colorComponents;
	int totalDataSize = 6 * pointAttr * sizeof(GLfloat);
    
    int quadIndex = 0;
    for(int i = 0; i < totalDataSize; i+=6) {
        compiledDataIndex->at(i)   = 0 + vertsPerQuad * quadIndex;
        compiledDataIndex->at(i+1) = 1 + vertsPerQuad * quadIndex;
        compiledDataIndex->at(i+2) = 3 + vertsPerQuad * quadIndex;
        
        compiledDataIndex->at(i+3) = 1 + vertsPerQuad * quadIndex;
        compiledDataIndex->at(i+4) = 2 + vertsPerQuad * quadIndex;
        compiledDataIndex->at(i+5) = 3 + vertsPerQuad * quadIndex;
        quadIndex++;
    }

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, compiledDataIndex->size() * sizeof(GLfloat), &compiledDataIndex->at(0), GL_STATIC_DRAW);
    
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, compiledData->size() * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(vertexAttribIndex, vertexComponents, GL_FLOAT, GL_FALSE, pointAttr * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(colorAttribIndex, colorComponents, GL_FLOAT, GL_FALSE, pointAttr * sizeof(GLfloat), (void*)(vertexComponents * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
    
    particlesShader = std::make_unique<Shader>("Shaders/vertex.vs", "Shaders/fragment.fs");
    
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH);

	std::cout << "Particle system initialized" << std::endl;
}

void particle_system::Emit()
{
    if (!looping) {
        size_t firstInactivePIndex = lastActiveParticle + 1;
        position->at(firstInactivePIndex) = particleAttr.position;
        velocity->at(firstInactivePIndex) = particleAttr.velocity;
        colorBegin->at(firstInactivePIndex) = particleAttr.colorBegin;
        colorEnd->at(firstInactivePIndex) = particleAttr.colorEnd;
        totalLife->at(firstInactivePIndex) = particleAttr.totalLife;
        lastActiveParticle++;
        
        std::cout << "Particle @ index " << lastActiveParticle << " created" << std::endl;
    }
    
    emitting = true;
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
        compiledData->clear();
        int compiledDataOffset = (vertexComponents + colorComponents) * vertsPerQuad;
        std::vector<float>::iterator dataIndex = compiledData->begin();
		for (int i = 0; i <= lastActiveParticle; i++) {
            float quadVertices[28] = {
                (*position)[i].x + 0.5f, (*position)[i].y + 0.5f, 0.0f, (*color)[i].r, (*color)[i].g, (*color)[i].b, (*color)[i].a,
                (*position)[i].x + 0.5f, (*position)[i].y - 0.5f, 0.0f, (*color)[i].r, (*color)[i].g, (*color)[i].b, (*color)[i].a,
                (*position)[i].x - 0.5f, (*position)[i].y - 0.5f, 0.0f, (*color)[i].r, (*color)[i].g, (*color)[i].b, (*color)[i].a,
                (*position)[i].x - 0.5f, (*position)[i].y + 0.5f, 0.0f, (*color)[i].r, (*color)[i].g, (*color)[i].b, (*color)[i].a,
            };
            if(dataIndex >= compiledData->end()) {
                compiledData->insert(dataIndex, &quadVertices[0], &quadVertices[0] + (sizeof(quadVertices)/sizeof(float)));
                std::advance(dataIndex, compiledDataOffset);
            }
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
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //glBufferData(GL_ARRAY_BUFFER, compiledData->size() * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, compiledData->size() * sizeof(GLfloat), &compiledData->at(0));
}

void particle_system::Update(timestep ts)
{
    if(!emitting) {
        return;
    }
    
	float delta = ts.GetSeconds();
    
    msElapsed += ts.GetMilliseconds();
    
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
            color->at(i) = glm::mix(colorEnd->at(i), colorBegin->at(i), remainingLife/particleAttr.totalLife);
        }
    }
    
    // Timed particle emission
    if(lastActiveParticle + 1 < totalParticles) {
        if(msElapsed >= (1000 / particleAttr.emissionRate) && looping) {
            size_t firstInactivePIndex = lastActiveParticle + 1;
            position->at(firstInactivePIndex) = particleAttr.position;
            velocity->at(firstInactivePIndex) = particleAttr.velocity;
            colorBegin->at(firstInactivePIndex) = particleAttr.colorBegin;
            colorEnd->at(firstInactivePIndex) = particleAttr.colorEnd;
            totalLife->at(firstInactivePIndex) = particleAttr.totalLife;
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

void particle_system::Render()
{
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    particlesShader->Bind();
    glDrawElements(GL_TRIANGLES, 6000, GL_UNSIGNED_INT, 0);
}
