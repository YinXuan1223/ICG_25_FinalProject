#pragma once
#include <vector>
#include <algorithm>
#include <glm/glm.hpp>
#include <glad/glad.h>

struct SplashParticle {
    glm::vec3 pos;
    float birth;
};

class SplashSystem {
public:
    GLuint VAO = 0, VBO = 0;
    std::vector<SplashParticle> particles;

    float life = 1.0f;
    size_t maxParticles = 20000;

    void init() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SplashParticle), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(SplashParticle), (void*)offsetof(SplashParticle, birth));

        glBindVertexArray(0);
    }

    void spawn(const glm::vec3& p, float t, int count = 1) {
        for (int i = 0; i < count; i++) particles.push_back({p, t});
        if (particles.size() > maxParticles) {
            particles.erase(particles.begin(), particles.begin() + (particles.size() - maxParticles));
        }
    }

    void cullDead(float now) {
        particles.erase(
            std::remove_if(particles.begin(), particles.end(),
                [&](const SplashParticle& sp){ return (now - sp.birth) > life; }),
            particles.end()
        );
    }

    void upload() {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(SplashParticle),
                     particles.data(), GL_DYNAMIC_DRAW);
    }

    void drawPoints() {
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, (GLsizei)particles.size());
        glBindVertexArray(0);
    }
};
