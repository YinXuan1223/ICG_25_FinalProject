#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <tiny_obj_loader.h>
#include "stb_image.h"

using namespace std;

enum class FACETYPE {
    TRIANGLE,
    QUAD
};

// 單個 Mesh 的資料結構
struct Mesh {
    vector<float> positions;
    vector<float> normals;
    vector<float> texcoords;
    
    unsigned int VAO;
    unsigned int VBO[3];
    int vertexCount;
    int materialID;  // 對應到 materials 陣列的 index
    
    void setupVAO() {
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);
        glGenBuffers(3, VBO);

        // Positions
        if (!positions.empty()) {
            glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * positions.size(), positions.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (void*)0);
            glEnableVertexAttribArray(0);
        }

        // Normals
        if (!normals.empty()) {
            glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * normals.size(), normals.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (void*)0);
            glEnableVertexAttribArray(1);
        }

        // Texture coordinates
        if (!texcoords.empty()) {
            glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * texcoords.size(), texcoords.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), (void*)0);
            glEnableVertexAttribArray(2);
        }

        glBindVertexArray(0);
        vertexCount = positions.size() / 3;
        
        // 清空資料節省記憶體
        positions.clear();
        normals.clear();
        texcoords.clear();
    }
};

// 材質資料結構
struct Material {
    string name;
    unsigned int textureID;
    bool hasTexture;
    
    Material() : textureID(0), hasTexture(false) {}
};

class Object {
public:
    vector<Mesh> meshes;
    vector<Material> materials;
    FACETYPE faceType = FACETYPE::TRIANGLE;
	string mtlbaseDirectory;
	string texDirectory;

    void draw() {
        for (const auto& mesh : meshes) {
            // 綁定該 mesh 對應的 texture
            if (mesh.materialID >= 0 && mesh.materialID < materials.size()) {
                const Material& mat = materials[mesh.materialID];
                if (mat.hasTexture) {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, mat.textureID);
                }
            }
            
            // 繪製 mesh
            glBindVertexArray(mesh.VAO);
            glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
        }
        glBindVertexArray(0);
    }

    Object(const string& filename, const string& mtlbase, const string& texbase) {
        
        texDirectory = texbase;
		mtlbaseDirectory = mtlbase;
        loadOBJ(filename, mtlbaseDirectory);
		
        
        // 為每個 mesh 設置 VAO
        for (auto& mesh : meshes) {
            mesh.setupVAO();
        }
    }

    ~Object() {
        // 清理資源
        for (auto& mesh : meshes) {
            glDeleteVertexArrays(1, &mesh.VAO);
            glDeleteBuffers(3, mesh.VBO);
        }
        for (auto& mat : materials) {
            if (mat.hasTexture) {
                glDeleteTextures(1, &mat.textureID);
            }
        }
    }

private:
    void loadOBJ(const string& filename, const string& mtlbase) {
        vector<tinyobj::shape_t> shapes;
        vector<tinyobj::material_t> objMaterials;
        string err, warn;

        // 載入 OBJ，並解析 MTL
        bool ret = tinyobj::LoadObj(shapes, objMaterials, err, filename.c_str(), mtlbase.c_str());

        if (!warn.empty()) {
            cout << "Warning: " << warn << endl;
        }

        if (!err.empty()) {
            cerr << "Error loading OBJ: " << err << endl;
        }

        if (!ret) {
            cerr << "Failed to load OBJ file: " << filename << endl;
            return;
        }

        // 載入所有材質和 texture
        loadMaterials(objMaterials);

        // 處理每個 shape (mesh)
        for (const auto& shape : shapes) {
            processMesh(shape);
        }

        cout << "Loaded " << meshes.size() << " meshes with " << materials.size() << " materials" << endl;
    }

    void loadMaterials(const vector<tinyobj::material_t>& objMaterials) {
        for (const auto& objMat : objMaterials) {
            Material mat;
            mat.name = objMat.name;

            // 檢查是否有 diffuse texture
            if (!objMat.diffuse_texname.empty()) {
                string texturePath = texDirectory + objMat.diffuse_texname;
				cout << "test texture path: " << texturePath << endl;
                mat.textureID = loadTexture(texturePath);
                mat.hasTexture = (mat.textureID != 0);
                
                if (mat.hasTexture) {
                    cout << "Loaded texture: " << texturePath << " for material: " << mat.name << endl;
                }
            }

            materials.push_back(mat);
        }

        // 如果沒有材質，創建一個預設材質
        if (materials.empty()) {
            materials.push_back(Material());
        }
    }

    void processMesh(const tinyobj::shape_t& shape) {
        const tinyobj::mesh_t& objMesh = shape.mesh;
        
        // 按材質 ID 分組處理 faces
        map<int, Mesh> meshByMaterial;

        size_t index_offset = 0;
        for (size_t f = 0; f < objMesh.num_vertices.size(); f++) {
            int fv = objMesh.num_vertices[f];
            int materialID = objMesh.material_ids.empty() ? -1 : objMesh.material_ids[f];
            
            // 如果材質 ID 無效，使用 0
            if (materialID < 0 || materialID >= materials.size()) {
                materialID = 0;
            }

            // 取得或創建對應材質的 mesh
            if (meshByMaterial.find(materialID) == meshByMaterial.end()) {
                meshByMaterial[materialID].materialID = materialID;
            }
            Mesh& currentMesh = meshByMaterial[materialID];

            // 處理三角形
            if (fv == 3) {
                addTriangle(currentMesh, objMesh, index_offset);
            } 
            // 處理四邊形
            else if (fv == 4) {
                addQuad(currentMesh, objMesh, index_offset);
                faceType = FACETYPE::QUAD;
            }

            index_offset += fv;
        }

        // 將所有分組的 mesh 加入 meshes 陣列
        for (auto& pair : meshByMaterial) {
            meshes.push_back(pair.second);
        }
		// 在 processMesh 加入這段
		cout << "Shape: " << shape.name << endl;
		cout << "  Vertices: " << objMesh.positions.size() / 3 << endl;
		cout << "  TexCoords: " << objMesh.texcoords.size() / 2 << endl;  // 應該 > 0
    }

    void addTriangle(Mesh& mesh, const tinyobj::mesh_t& objMesh, size_t offset) {
        for (size_t v = 0; v < 3; v++) {
            unsigned int idx = objMesh.indices[offset + v];
            addVertex(mesh, objMesh, idx);
        }
    }

    void addQuad(Mesh& mesh, const tinyobj::mesh_t& objMesh, size_t offset) {
        vector<unsigned int> quad_indices;
        for (size_t v = 0; v < 4; v++) {
            quad_indices.push_back(objMesh.indices[offset + v]);
        }

        // 第一個三角形: 0, 1, 2
        for (int i : {0, 1, 2}) {
            addVertex(mesh, objMesh, quad_indices[i]);
        }

        // 第二個三角形: 0, 2, 3
        for (int i : {0, 2, 3}) {
            addVertex(mesh, objMesh, quad_indices[i]);
        }
    }

    void addVertex(Mesh& mesh, const tinyobj::mesh_t& objMesh, unsigned int idx) {
        // Positions
        if (idx * 3 + 2 < objMesh.positions.size()) {
            mesh.positions.push_back(objMesh.positions[idx * 3 + 0]);
            mesh.positions.push_back(objMesh.positions[idx * 3 + 1]);
            mesh.positions.push_back(objMesh.positions[idx * 3 + 2]);
        } else {
            mesh.positions.insert(mesh.positions.end(), {0.0f, 0.0f, 0.0f});
        }

        // Texture coordinates
        if (!objMesh.texcoords.empty() && idx * 2 + 1 < objMesh.texcoords.size()) {
            mesh.texcoords.push_back(objMesh.texcoords[idx * 2 + 0]);
            mesh.texcoords.push_back(objMesh.texcoords[idx * 2 + 1]);
        } else {
            mesh.texcoords.insert(mesh.texcoords.end(), {0.0f, 0.0f});
        }

        // Normals
        if (!objMesh.normals.empty() && idx * 3 + 2 < objMesh.normals.size()) {
            mesh.normals.push_back(objMesh.normals[idx * 3 + 0]);
            mesh.normals.push_back(objMesh.normals[idx * 3 + 1]);
            mesh.normals.push_back(objMesh.normals[idx * 3 + 2]);
        } else {
            mesh.normals.insert(mesh.normals.end(), {0.0f, 1.0f, 0.0f});
        }
    }

    unsigned int loadTexture(const string& filepath) {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &nrChannels, 0);
        
        if (data) {
            GLenum format = GL_RGB;
            if (nrChannels == 1) format = GL_RED;
            else if (nrChannels == 3) format = GL_RGB;
            else if (nrChannels == 4) format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            cerr << "Failed to load texture: " << filepath << endl;
            stbi_image_free(data);
            return 0;
        }
        
        stbi_image_free(data);
        return textureID;
    }
};
// TACODE
// #include <vector>
// #include <string>
// #include <iostream>
// #include <glm/glm.hpp>
// #include <glad/glad.h>
// #include <tiny_obj_loader.h>
// #include "stb_image.h"

// using namespace std;

// enum class FACETYPE
// {
// 	TRIANGLE,
// 	QUAD
// };

// class Object
// {
// public:
// 	vector<float> positions;
// 	vector<float> normals;
// 	vector<float> texcoords;
// 	FACETYPE faceType = FACETYPE::TRIANGLE;

// 	void draw(){
// 		if(hasTexture){
// 			glActiveTexture(GL_TEXTURE0);
// 			glBindTexture(GL_TEXTURE_2D, textureID);
// 		}
// 		glBindVertexArray(VAO);
// 		glDrawArrays(GL_TRIANGLES, 0, vertex_cnt);
// 	}

// 	Object(const string& filename)
// 	{
// 		loadOBJ(filename);
// 		set_VAO();
// 	}

// 	void loadTexture(const string& filepath){
// 		glGenTextures(1, &textureID);
// 		glBindTexture(GL_TEXTURE_2D, textureID);

// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

// 		int width, height, nrChannels;
//         stbi_set_flip_vertically_on_load(true);
// 		unsigned char *data = stbi_load(filepath.c_str(), &width, &height, &nrChannels, 0);
// 		if (data) {
// 			GLenum format = GL_RGB;
// 			if (nrChannels == 1) format = GL_RED;
// 			else if (nrChannels == 3) format = GL_RGB;
// 			else if (nrChannels == 4) format = GL_RGBA;

// 			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
// 			glGenerateMipmap(GL_TEXTURE_2D);
// 			hasTexture = true;
// 		} else {
// 			std::cerr << "Failed to load texture: " << filepath << std::endl;
// 		}
// 		stbi_image_free(data);
// 	}

// private:
// 	unsigned int VAO;
// 	unsigned int textureID = 0;
// 	bool hasTexture = false;
// 	int vertex_cnt;

// 	void loadOBJ(const string& filename) {
// 		vector<tinyobj::shape_t> shapes;
// 		vector<tinyobj::material_t> materials;
// 		string err;

// 		bool ret = tinyobj::LoadObj(shapes, materials, err, filename.c_str());

// 		if (!err.empty()) {
// 			cerr << "Error loading OBJ: " << err << endl;
// 		}

// 		if (!ret) {
// 			cerr << "Failed to load OBJ file: " << filename << endl;
// 			return;
// 		}

// 		// Process all shapes
// 		for (const auto& shape : shapes) {
// 			const tinyobj::mesh_t& mesh = shape.mesh;
			
// 			// Check if we have quads by examining face vertex counts
// 			for (size_t i = 0; i < mesh.num_vertices.size(); i++) {
// 				if (mesh.num_vertices[i] == 4) {
// 					faceType = FACETYPE::QUAD;
// 					break;
// 				}
// 			}

// 			// Process faces
// 			size_t index_offset = 0;
// 			for (size_t f = 0; f < mesh.num_vertices.size(); f++) {
// 				int fv = mesh.num_vertices[f];
				
// 				if (fv == 3) {
// 					// Triangle
// 					for (size_t v = 0; v < 3; v++) {
// 						unsigned int idx = mesh.indices[index_offset + v];
						
// 						// Positions
// 						if (idx * 3 + 2 < mesh.positions.size()) {
// 							positions.push_back(mesh.positions[idx * 3 + 0]);
// 							positions.push_back(mesh.positions[idx * 3 + 1]);
// 							positions.push_back(mesh.positions[idx * 3 + 2]);
// 						} else {
// 							positions.push_back(0.0f);
// 							positions.push_back(0.0f);
// 							positions.push_back(0.0f);
// 						}

// 						// Texture coordinates
// 						if (!mesh.texcoords.empty() && idx * 2 + 1 < mesh.texcoords.size()) {
// 							texcoords.push_back(mesh.texcoords[idx * 2 + 0]);
// 							texcoords.push_back(mesh.texcoords[idx * 2 + 1]);
// 						} else {
// 							texcoords.push_back(0.0f);
// 							texcoords.push_back(0.0f);
// 						}

// 						// Normals
// 						if (!mesh.normals.empty() && idx * 3 + 2 < mesh.normals.size()) {
// 							normals.push_back(mesh.normals[idx * 3 + 0]);
// 							normals.push_back(mesh.normals[idx * 3 + 1]);
// 							normals.push_back(mesh.normals[idx * 3 + 2]);
// 						} else {
// 							normals.push_back(0.0f);
// 							normals.push_back(1.0f);
// 							normals.push_back(0.0f);
// 						}
// 					}
// 				} else if (fv == 4) {
// 					// Quad - convert to two triangles
// 					vector<unsigned int> quad_indices;
// 					for (size_t v = 0; v < 4; v++) {
// 						quad_indices.push_back(mesh.indices[index_offset + v]);
// 					}

// 					// First triangle: 0, 1, 2
// 					for (int i : {0, 1, 2}) {
// 						unsigned int idx = quad_indices[i];
						
// 						// Positions
// 						if (idx * 3 + 2 < mesh.positions.size()) {
// 							positions.push_back(mesh.positions[idx * 3 + 0]);
// 							positions.push_back(mesh.positions[idx * 3 + 1]);
// 							positions.push_back(mesh.positions[idx * 3 + 2]);
// 						} else {
// 							positions.push_back(0.0f);
// 							positions.push_back(0.0f);
// 							positions.push_back(0.0f);
// 						}

// 						// Texture coordinates
// 						if (!mesh.texcoords.empty() && idx * 2 + 1 < mesh.texcoords.size()) {
// 							texcoords.push_back(mesh.texcoords[idx * 2 + 0]);
// 							texcoords.push_back(mesh.texcoords[idx * 2 + 1]);
// 						} else {
// 							texcoords.push_back(0.0f);
// 							texcoords.push_back(0.0f);
// 						}

// 						// Normals
// 						if (!mesh.normals.empty() && idx * 3 + 2 < mesh.normals.size()) {
// 							normals.push_back(mesh.normals[idx * 3 + 0]);
// 							normals.push_back(mesh.normals[idx * 3 + 1]);
// 							normals.push_back(mesh.normals[idx * 3 + 2]);
// 						} else {
// 							normals.push_back(0.0f);
// 							normals.push_back(1.0f);
// 							normals.push_back(0.0f);
// 						}
// 					}

// 					// Second triangle: 0, 2, 3
// 					for (int i : {0, 2, 3}) {
// 						unsigned int idx = quad_indices[i];
						
// 						// Positions
// 						if (idx * 3 + 2 < mesh.positions.size()) {
// 							positions.push_back(mesh.positions[idx * 3 + 0]);
// 							positions.push_back(mesh.positions[idx * 3 + 1]);
// 							positions.push_back(mesh.positions[idx * 3 + 2]);
// 						} else {
// 							positions.push_back(0.0f);
// 							positions.push_back(0.0f);
// 							positions.push_back(0.0f);
// 						}

// 						// Texture coordinates
// 						if (!mesh.texcoords.empty() && idx * 2 + 1 < mesh.texcoords.size()) {
// 							texcoords.push_back(mesh.texcoords[idx * 2 + 0]);
// 							texcoords.push_back(mesh.texcoords[idx * 2 + 1]);
// 						} else {
// 							texcoords.push_back(0.0f);
// 							texcoords.push_back(0.0f);
// 						}

// 						// Normals
// 						if (!mesh.normals.empty() && idx * 3 + 2 < mesh.normals.size()) {
// 							normals.push_back(mesh.normals[idx * 3 + 0]);
// 							normals.push_back(mesh.normals[idx * 3 + 1]);
// 							normals.push_back(mesh.normals[idx * 3 + 2]);
// 						} else {
// 							normals.push_back(0.0f);
// 							normals.push_back(1.0f);
// 							normals.push_back(0.0f);
// 						}
// 					}
// 				}
				
// 				index_offset += fv;
// 			}
// 		}
// 	}

// 	void set_VAO(){
// 		unsigned int VBO[3];
// 		glGenVertexArrays(1, &VAO);
// 		glBindVertexArray(VAO);
// 		glGenBuffers(3, VBO);

// 		// Positions
// 		if (!positions.empty()) {
// 			glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
// 			glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * positions.size(), positions.data(), GL_STATIC_DRAW);
// 			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (void*)0);
// 			glEnableVertexAttribArray(0);
// 		}

// 		// Normals
// 		if (!normals.empty()) {
// 			glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
// 			glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * normals.size(), normals.data(), GL_STATIC_DRAW);
// 			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (void*)0);
// 			glEnableVertexAttribArray(1);
// 		}

// 		// Texture coordinates
// 		if (!texcoords.empty()) {
// 			glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
// 			glBufferData(GL_ARRAY_BUFFER, sizeof(GL_FLOAT) * texcoords.size(), texcoords.data(), GL_STATIC_DRAW);
// 			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GL_FLOAT), (void*)0);
// 			glEnableVertexAttribArray(2);
// 		}

// 		glBindVertexArray(0);

// 		vertex_cnt = positions.size() / 3;
		
// 		// Clear vectors to save memory after uploading to GPU
// 		positions.clear();
// 		texcoords.clear();
// 		normals.clear();
// 	}
// };
