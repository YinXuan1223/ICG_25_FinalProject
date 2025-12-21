#include <glad/glad.h>
#include <vector>

class WaterPlane {
public:
    WaterPlane(float size = 1000.0f, int divisions = 50) {
        generatePlane(size, divisions);
        setupBuffers();
    }

    ~WaterPlane() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    void draw() {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

private:
    unsigned int VAO, VBO, EBO;
    int indexCount;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    void generatePlane(float size, int divisions) {
        float halfSize = size / 2.0f;
        float step = size / divisions;

        // 生成頂點（position + normal + texcoord）
        for (int z = 0; z <= divisions; z++) {
            for (int x = 0; x <= divisions; x++) {
                float xPos = -halfSize + x * step;
                float zPos = -halfSize + z * step;
                
                // Position
                vertices.push_back(xPos);
                vertices.push_back(0.0f);  // Y = 0，水平面
                vertices.push_back(zPos);
                
                // Normal (向上)
                vertices.push_back(0.0f);
                vertices.push_back(1.0f);
                vertices.push_back(0.0f);
                
                // TexCoord
                vertices.push_back((float)x / divisions);
                vertices.push_back((float)z / divisions);
            }
        }

        // 生成索引
        for (int z = 0; z < divisions; z++) {
            for (int x = 0; x < divisions; x++) {
                int topLeft = z * (divisions + 1) + x;
                int topRight = topLeft + 1;
                int bottomLeft = (z + 1) * (divisions + 1) + x;
                int bottomRight = bottomLeft + 1;

                // 第一個三角形
                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                // 第二個三角形
                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }

        indexCount = indices.size();
    }

    void setupBuffers() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // TexCoord attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }
};