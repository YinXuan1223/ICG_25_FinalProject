#include <bits/stdc++.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "header/cube.h"
#include "header/Object.h"
#include "header/shader.h"
#include "header/stb_image.h"
#include "header/waterplane.h"

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow *window);
void updateCamera();
void applyOrbitDelta(float yawDelta, float pitchDelta, float radiusDelta);
unsigned int loadCubemap(std::vector<std::string> &mFileName);


struct camera_t{
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;
    glm::vec3 target;

    float yaw;
    float pitch;
    float radius;
    float minRadius;
    float maxRadius;
    float orbitRotateSpeed;
    float orbitZoomSpeed;
    float minOrbitPitch;
    float maxOrbitPitch;
    bool enableAutoOrbit;
    float autoOrbitSpeed;
};

// settings
int SCR_WIDTH = 800;
int SCR_HEIGHT = 600;


// shader programs 
shader_program_t* carShader;
shader_program_t* waterplaneShader;


camera_t camera;

Object* carModel = nullptr;
Object* cubeModel = nullptr;



// waterplane
WaterPlane* waterplaneModel = nullptr;


// Mcqueen Status
float car_speed = 3.0f;
glm::vec3 car_position = glm::vec3(0.0f, 0.0f, 0.0f);

float currentTime = 0.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void model_setup(){
#if defined(__linux__) || defined(__APPLE__)
    std::string obj_path = "..\\..\\src\\asset\\obj\\Mei_Run.obj";
    std::string cube_obj_path = "..\\..\\src\\asset\\obj\\cube.obj";
    std::string texture_path = "..\\..\\src\\asset\\texture\\Mei_TEX.png";
#else
    std::string obj_path = "..\\..\\src\\asset\\obj\\LMQ.obj";
    std::string mtlbase_path = "..\\..\\src\\asset\\material\\";
    std::string texbase_path = "..\\..\\src\\asset\\texture\\LMQ\\";

    std::string cube_obj_path = "..\\..\\src\\asset\\obj\\cube.obj";
#endif

    carModel = new Object(obj_path, mtlbase_path, texbase_path);
    waterplaneModel = new WaterPlane(1000, 50);


}

void camera_setup(){
    camera.worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    camera.yaw = 90.0f;
    camera.pitch = 10.0f;
    camera.radius = 400.0f;
    camera.minRadius = 150.0f;
    camera.maxRadius = 800.0f;
    // camera.orbitRotateSpeed = 60.0f;
    camera.orbitRotateSpeed = 0.0f;
    // camera.orbitZoomSpeed = 400.0f;
    camera.orbitZoomSpeed = 0.0f;
    camera.minOrbitPitch = -80.0f;
    camera.maxOrbitPitch = 80.0f;
    camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
    camera.enableAutoOrbit = false;
    // camera.autoOrbitSpeed = 20.0f;
    camera.autoOrbitSpeed = 0.0f;

    updateCamera();
}

void updateCamera(){
    float yawRad = glm::radians(camera.yaw);
    float pitchRad = glm::radians(camera.pitch);
    float cosPitch = cos(pitchRad);

    camera.position.x = camera.target.x + camera.radius * cosPitch * cos(yawRad);
    camera.position.y = camera.target.y + camera.radius * sin(pitchRad);
    camera.position.z = camera.target.z + camera.radius * cosPitch * sin(yawRad);

    camera.front = glm::normalize(camera.target - camera.position);
    camera.right = glm::normalize(glm::cross(camera.front, camera.worldUp));
    camera.up = glm::normalize(glm::cross(camera.right, camera.front));
}

void applyOrbitDelta(float yawDelta, float pitchDelta, float radiusDelta) {
    camera.yaw += yawDelta;
    camera.pitch = glm::clamp(camera.pitch + pitchDelta, camera.minOrbitPitch, camera.maxOrbitPitch);
    camera.radius = glm::clamp(camera.radius + radiusDelta, camera.minRadius, camera.maxRadius);
    updateCamera();
}



void shader_setup(){
#if defined(__linux__) || defined(__APPLE__)
    std::string shaderDir = "..\\..\\src\\shaders\\";
#else
    std::string shaderDir = "..\\..\\src\\shaders\\";
#endif

    // setup waterplane shader
    std::string vpath = shaderDir + "waterplane.vert";
    std::string fpath = shaderDir + "waterplane.frag";
    waterplaneShader = new shader_program_t();
    waterplaneShader->create();
    waterplaneShader->add_shader(vpath, GL_VERTEX_SHADER);
    waterplaneShader->add_shader(fpath, GL_FRAGMENT_SHADER);
    waterplaneShader->link_shader();

    // setup car shader
    vpath = shaderDir + "car.vert";
    fpath = shaderDir + "car.frag";

    carShader = new shader_program_t();
    carShader->create();
    carShader->add_shader(vpath, GL_VERTEX_SHADER);
    carShader->add_shader(fpath, GL_FRAGMENT_SHADER);
    carShader->link_shader();

}



void setup(){

    model_setup();
    shader_setup();
    camera_setup();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
}

void update(){
    currentTime = glfwGetTime();
    deltaTime = currentTime - lastFrame;
    lastFrame = currentTime;

    if (camera.enableAutoOrbit) {
        float yawDelta = camera.autoOrbitSpeed * deltaTime;
        applyOrbitDelta(yawDelta, 0.0f, 0.0f);
    }
    
    car_position.x += car_speed * deltaTime;

    // Position camera behind the car for a chase view.
    const glm::vec3 carDirection = glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f));
    const float followDistance = 150.0f;
    const float heightOffset = 45.0f;
    const float lookAhead = 25.0f;

    camera.target = car_position + carDirection * lookAhead;
    camera.position = car_position - carDirection * followDistance + glm::vec3(0.0f, heightOffset, 0.0f);
    camera.front = glm::normalize(camera.target - camera.position);
    camera.right = glm::normalize(glm::cross(camera.front, camera.worldUp));
    camera.up = glm::normalize(glm::cross(camera.right, camera.front));

}

void render(){
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render car
    glm::mat4 view = glm::lookAt(camera.position, car_position, camera.up);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
    glm::mat4 carMatrix(1.0f);
    carMatrix = glm::translate(carMatrix, car_position);
    carMatrix = glm::rotate(carMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    carMatrix = glm::scale(carMatrix, glm::vec3(20.0f));
    carShader->use();

    carShader->set_uniform_value("model", carMatrix);
    carShader->set_uniform_value("view", view);
    carShader->set_uniform_value("projection", projection);

    carShader->set_uniform_value("ourTexture", 0);
    

    carModel->draw();
    carShader->release();

    // render waterplane
    glm::mat4 waterMatrix(1.0f);
    waterMatrix = glm::scale(waterMatrix, glm::vec3(200.0f, 1.0f, 200.0f));
    waterplaneShader->use();
    waterplaneShader->set_uniform_value("model", waterMatrix);
    waterplaneShader->set_uniform_value("view", view);
    waterplaneShader->set_uniform_value("projection", projection);

    // waterplaneShader->set_uniform_value("cameraPos", camera.position - glm::vec3(0.0f, 0.2f, 0.1f));
    
    waterplaneShader->set_uniform_value("waterColor", glm::vec3(0.0f, 0.35f, 0.45f));
    waterplaneShader->set_uniform_value("fogColor",   glm::vec3(0.6f, 0.7f, 0.8f));
    waterplaneShader->set_uniform_value("maxDist", 120.0f);
    waterplaneShader->set_uniform_value("fadeWidth", 30.0f);

    waterplaneModel->draw();
    waterplaneShader->release();


    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);


    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "HW3-Static Model", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwGetFramebufferSize(window, &SCR_WIDTH, &SCR_HEIGHT);
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    setup();
    
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        update(); 
        render(); 
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete carModel;
    delete waterplaneModel;
    
    delete carShader;
    delete waterplaneShader;

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    glm::vec2 orbitInput(0.0f);
    float zoomInput = 0.0f;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        orbitInput.x += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        orbitInput.x -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        orbitInput.y += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        orbitInput.y -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        zoomInput -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        zoomInput += 1.0f;

    if (orbitInput.x != 0.0f || orbitInput.y != 0.0f || zoomInput != 0.0f) {
        float yawDelta = orbitInput.x * camera.orbitRotateSpeed * deltaTime;
        float pitchDelta = orbitInput.y * camera.orbitRotateSpeed * deltaTime;
        float radiusDelta = zoomInput * camera.orbitZoomSpeed * deltaTime;
        applyOrbitDelta(yawDelta, pitchDelta, radiusDelta);
    }
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    (void)window;
    (void)scancode;
    (void)mods;

    if (action != GLFW_PRESS)
        return;

    if (key == GLFW_KEY_O) {
        camera.enableAutoOrbit = !camera.enableAutoOrbit;
        if (camera.enableAutoOrbit && camera.autoOrbitSpeed == 0.0f)
            camera.autoOrbitSpeed = 20.0f;
        else if (!camera.enableAutoOrbit)
            camera.autoOrbitSpeed = 0.0f;
    }
}


void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

