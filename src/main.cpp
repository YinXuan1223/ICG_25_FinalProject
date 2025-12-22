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
#include "header/splash.h"



void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);


struct camera_t {
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
    glm::vec3 worldUp;
};


int SCR_WIDTH  = 800;
int SCR_HEIGHT = 600;

shader_program_t* carShader        = nullptr;
shader_program_t* waterplaneShader = nullptr;
shader_program_t* splashShader     = nullptr;

camera_t camera;

Object*     carModel        = nullptr;
WaterPlane* waterplaneModel = nullptr;
SplashSystem* splash = nullptr;

// car status
float car_speed      = 150.0f;               
float car_turn_speed = glm::radians(90.0f);  
glm::vec3 car_position = glm::vec3(0.0f);
glm::vec3 car_velocity = glm::vec3(0.0f);
float     car_rotation = 0.0f;                

// About timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void model_setup() {
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

    carModel        = new Object(obj_path, mtlbase_path, texbase_path);
    waterplaneModel = new WaterPlane(1000, 50);
}

void camera_setup() {
    camera.worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    camera.position = glm::vec3(0.0f, 400.0f, 400.0f);
    camera.target   = glm::vec3(0.0f);
    camera.up       = glm::vec3(0.0f, 1.0f, 0.0f);
}

void shader_setup() {
    std::string shaderDir = "..\\..\\src\\shaders\\";
    std::string vpath = shaderDir + "waterplane.vert";
    std::string fpath = shaderDir + "waterplane.frag";

    // water shader
    waterplaneShader = new shader_program_t();
    waterplaneShader->create();
    waterplaneShader->add_shader(vpath, GL_VERTEX_SHADER);
    waterplaneShader->add_shader(fpath, GL_FRAGMENT_SHADER);
    waterplaneShader->link_shader();

    // car shader
    vpath = shaderDir + "car.vert";
    fpath = shaderDir + "car.frag";
    carShader = new shader_program_t();
    carShader->create();
    carShader->add_shader(vpath, GL_VERTEX_SHADER);
    carShader->add_shader(fpath, GL_FRAGMENT_SHADER);
    carShader->link_shader();

    std::string gpath = shaderDir + "splash.geom";
    vpath = shaderDir + "splash.vert";
    fpath = shaderDir + "splash.frag";
    splashShader = new shader_program_t();
    splashShader->create();
    splashShader->add_shader(vpath, GL_VERTEX_SHADER);
    splashShader->add_shader(fpath, GL_FRAGMENT_SHADER);
    splashShader->add_shader(gpath, GL_GEOMETRY_SHADER);
    splashShader->link_shader();
}

void setup() {
    model_setup();
    shader_setup();
    camera_setup();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    splash = new SplashSystem();
    splash->init();
}


void update() {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    car_position += car_velocity * deltaTime;

    float waterY = 0.0f; 
    bool onWater = (car_position.y <= waterY + 0.05f);

    if (onWater) {
        glm::vec3 wheelL = car_position + glm::vec3( 15.0f, 0.5f,  0.0f);
        glm::vec3 wheelR = car_position + glm::vec3(-15.0f, 0.5f,  0.0f);

        splash->spawn(wheelL, currentFrame, 2);
        splash->spawn(wheelR, currentFrame, 2);

    }

    splash->cullDead(currentFrame);
    splash->upload();
}

void render() {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = glm::lookAt(
        camera.position,
        camera.target,
        camera.up
    );

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (float)SCR_WIDTH / SCR_HEIGHT,
        0.1f,
        1000.0f
    );

    // rendering car
    glm::mat4 carMatrix(1.0f);
    carMatrix = glm::translate(carMatrix, car_position);
    carMatrix = glm::rotate(carMatrix, car_rotation, glm::vec3(0.0f, 1.0f, 0.0f));
    carMatrix = glm::scale(carMatrix, glm::vec3(20.0f));

    carShader->use();
    carShader->set_uniform_value("model", carMatrix);
    carShader->set_uniform_value("view", view);
    carShader->set_uniform_value("projection", projection);
    carShader->set_uniform_value("ourTexture", 0);
    carModel->draw();
    carShader->release();

    // rendering water
    glm::mat4 waterMatrix(1.0f);
    waterMatrix = glm::scale(waterMatrix, glm::vec3(200.0f, 1.0f, 200.0f));

    waterplaneShader->use();
    waterplaneShader->set_uniform_value("model", waterMatrix);
    waterplaneShader->set_uniform_value("view", view);
    waterplaneShader->set_uniform_value("projection", projection);
    waterplaneShader->set_uniform_value("waterColor", glm::vec3(0.6f, 0.7f, 0.8f));
    waterplaneModel->draw();
    waterplaneShader->release();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 建議：透明物件不要寫入 depth（避免怪遮擋）
    glDepthMask(GL_FALSE);

    // 建議：避免被背面剔除
    glDisable(GL_CULL_FACE);

    splashShader->use();
    splashShader->set_uniform_value("view", view);
    splashShader->set_uniform_value("projection", projection);
    splashShader->set_uniform_value("cameraPos", camera.position);
    splashShader->set_uniform_value("time", (float)glfwGetTime());
    splashShader->set_uniform_value("life", 0.5f);

    splash->drawPoints();

    splashShader->release();

    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

}


void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        car_rotation += car_turn_speed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        car_rotation -= car_turn_speed * deltaTime;


    
    glm::vec3 forward;
    forward.z = cos(car_rotation);
    forward.x = sin(car_rotation);
    forward = glm::normalize(forward);

    car_velocity = glm::vec3(0.0f);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        car_velocity += forward * car_speed;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        car_velocity -= forward * car_speed;
}


void framebufferSizeCallback(GLFWwindow*, int width, int height) {
    SCR_WIDTH  = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
}


int main() {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(
        SCR_WIDTH,
        SCR_HEIGHT,
        "HW4 group 12",
        nullptr,
        nullptr
    );

    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n";
        return -1;
    }

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
    delete splashShader;


    glfwTerminate();
    return 0;
}
