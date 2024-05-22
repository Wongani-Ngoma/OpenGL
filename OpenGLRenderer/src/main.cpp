#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <fstream>
#include <sstream>
#include <streambuf>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "graphics/shader.h"
#include "graphics/texture.h"
#include "graphics/models/cube.hpp"
#include "graphics/models/lamp.hpp"
#include "graphics/light.h"
#include "graphics/model.h"

#include "../io/Keyboard.h"
#include "../io/Joystick.h"
#include "../io/Mouse.h"
#include "../io/Camera.h"
#include "../io/screen.h"

void processInput(double dt);

float mixVal = 1.0f;
float zoomFactor = 1.0f;

glm::mat4 transform = glm::mat4(1.0f);
Joystick mainJ(0);

Camera cameras[2]{
    Camera(glm::vec3(0.0f, 0.0f, 2.0f)),
    Camera(glm::vec3(10.0f, 10.0f, 10.0f))
};

int activeCamera = 0;
float deltaTime;
float lastFrame = 0.0f;

float sensitivity = 0.2f; // of controls
float spotLightStrength = 10;

bool flashlightOn = true;

Screen screen;

int err;

int main() {

    int success;
    char infoLog[512];

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true); //for debugging
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__ //For macOS
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPACT, GL_TRUE);
#endif

    if (!screen.init()) {
        std::cout << "Could not create window\n";
        glfwTerminate();
        return -1;
        
    }

    //Glad needs to be initilized before any OpenGL calls
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initilize glad\n";
        glfwTerminate();
        return -1;
    }

    screen.setParameters();

    glEnable(GL_DEPTH_TEST);

    //Shaders
    Shader shader = Shader("assets/object.vert", "assets/object.frag");
    Shader lampShader = Shader("assets/object.vert", "assets/lamp.frag");

    Model container(glm::vec3(0.0f), glm::vec3(1.0), true);
    container.loadModel("assets\\models\\container\\source\\container.obj");
    Model m(glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.5), true);
    m.loadModel("assets\\models\\ak-47\\source\\ak-47.obj"); // path of model

    DirLight dirLight = { glm::vec3(-0.2, -1.0, -0.3),
        glm::vec4(0.1, 0.1, 0.1, 1.0),
        glm::vec4(0.4, 0.4, 0.4, 1.0f), 
        glm::vec4(0.75, 0.75, 0.75, 1.0f) 
    };

    glm::vec3 pointLightPositions[] = {
            glm::vec3(0.7f,  0.2f,  2.0f),
            glm::vec3(2.3f, -3.3f, -4.0f),
            glm::vec3(-4.0f,  2.0f, -12.0f),
    };

    Lamp lamps[3];
    for (int i = 0; i < 3; i++) {
        lamps[i] = Lamp( 
            glm::vec3(50.0f, 50.0f, 130.0f),
            glm::vec4(1.0, 1.0, 1.0, 1.0),
            glm::vec4(1.0, 1.0, 1.0, 1.0),
            glm::vec4(1.0, 1.0, 1.0, 1.0),
            1.0, 0.07, 0.032,
            pointLightPositions[i],
            glm::vec3(1.0)
        );
        lamps[i].material = Material::gold;
        lamps[i].init();
    }

    SpotLight spotLight = {
        cameras[activeCamera].cameraPos, cameras[activeCamera].cameraFront,
        glm::cos(glm::radians(12.5)), glm::cos(glm::radians(20.0)),
        1.0f, 0.07f, 0.032f,
        glm::vec4(0.0, 0.0, 0.0, 1.0f), glm::vec4(2.0, 2.0, 2.0, 1.0), glm::vec4(2.0, 2.0, 2.0, 1.0)
    };

    mainJ.update();
    if (mainJ.isPresent()) {
        std::cout << mainJ.getName() << " is present.\n";
    }
    else {
        std::cout << "No controller present.\n";
    }

    glEnable(GL_DEPTH_TEST);

    while (!screen.shouldClose()) {

        double currentTime = glfwGetTime();
        deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;

        //Process input
        processInput(deltaTime);

        //Render
        screen.update();

        shader.activate();
        shader.set3Float("viewPos", cameras[activeCamera].cameraPos);
        
        /*
        * rotating the directional light like the sun travelling through the sky
        dirLight.direction = glm::vec3(
            glm::rotate(glm::mat4(1.0f), (float)glm::radians(0.5f), glm::vec3(1.0f, 0.0f, 0.0f)) *
            glm::vec4(dirLight.direction, 1.0f));
        dirLight.render(shader);
        */

        shader.setInt("noPointLights", 3);
        for (int i = 0; i < 3; i++) {
            lamps[i].pointLight.render(shader, i);
        }
    
        if (flashlightOn) {
            spotLight.position = cameras[activeCamera].cameraPos;
            spotLight.direction = cameras[activeCamera].cameraFront;
            spotLight.render(shader, 0);
            shader.setInt("noSpotLights", 1);
        }
        else {
            shader.setInt("noSpotLights", 0);
        }


        //Create transformation for screen
        glm::mat4 view = glm::mat4(1.0f); // from world to camera, specific to each camera
        glm::mat4 projection = glm::mat4(1.0); // general, actually specific to each viewport
        view = cameras[activeCamera].getViewMatrix();
        projection = glm::perspective(glm::radians(cameras[activeCamera].getZoom()), (float)screen.SCR_WIDTH / (float)screen.SCR_HEIGHT, 0.1f, 100.0f);

        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        for (int i = 0; i < 3; i++) {
            lamps[i].render(shader);
        }

        container.render(shader);
        m.render(shader);
        
        lampShader.activate();
        lampShader.setMat4("view", view);
        lampShader.setMat4("projection", projection);
        
        //Send new frame to window
        screen.newFrame();

        err = glGetError();
        if (err) {
            std::cout << err << "\n";
            //break;
        }
    }

    container.cleanup();
    m.cleanup();
    
    for (int i = 0; i < 3; i++) {
        lamps[i].cleanup();
    }

    glfwTerminate();
    return 0;
}



void processInput(double dt) {

    mainJ.update();
    if (Keyboard::key(GLFW_KEY_ESCAPE) || mainJ.buttonState(GLFW_JOYSTICK_START)) {
        screen.setShouldClose(true);
    }

    if (Keyboard::key(GLFW_KEY_LEFT_ALT) && Keyboard::keyWentDown(GLFW_KEY_ENTER)) {
        screen.toogleFullScreenWindowed();
    }

    if (Keyboard::key(GLFW_KEY_W)) {
        mixVal += 0.005;
        if (mixVal > 1.0f) {
            mixVal = 1.0f;
        }
    }

    if (Keyboard::key(GLFW_KEY_S)) {
        mixVal -= 0.005;
        if (mixVal < 0.0f) {
            mixVal = 0.0f;
        }
    }

    if (Keyboard::keyWentDown(GLFW_KEY_TAB)) {
        activeCamera = !activeCamera;  
    }

    // move camera
    if (Keyboard::key(GLFW_KEY_UP)) {
        cameras[activeCamera].updateCameraPos(CameraDirection::FORWARD, dt);
    }
    if (Keyboard::key(GLFW_KEY_DOWN)) {
        cameras[activeCamera].updateCameraPos(CameraDirection::BACKWARD, dt);
    }
    if (Keyboard::key(GLFW_KEY_LEFT)) {
        cameras[activeCamera].updateCameraPos(CameraDirection::LEFT, dt);
    }
    if (Keyboard::key(GLFW_KEY_RIGHT)) {
        cameras[activeCamera].updateCameraPos(CameraDirection::RIGHT, dt);
    }
    if (Keyboard::key(GLFW_KEY_SPACE)) {
        cameras[activeCamera].updateCameraPos(CameraDirection::UP, dt);
    }
    if (Keyboard::key(GLFW_KEY_LEFT_SHIFT)) {
        cameras[activeCamera].updateCameraPos(CameraDirection::DOWN, dt);
    }

    // toogle spotlight on/off
    if (Keyboard::keyWentDown(GLFW_KEY_F)) {
        flashlightOn = !flashlightOn;
    }

    //keyboard for direction
    if (Keyboard::key(GLFW_KEY_W)) {
        cameras[activeCamera].updateCameraDirection(0.0, 0.25);
    }
    if (Keyboard::key(GLFW_KEY_S)) {
        cameras[activeCamera].updateCameraDirection(0.0, -0.25);
    }
    if (Keyboard::key(GLFW_KEY_A)) {
        cameras[activeCamera].updateCameraDirection(-0.25, 0.0);
    }
    if (Keyboard::key(GLFW_KEY_D)) {
        cameras[activeCamera].updateCameraDirection(0.25, 0.0);
    }

    //Mouse for direction
    double dx = Mouse::getDX(), dy = Mouse::getDY();
    if (dx != 0 || dy != 0) {
        cameras[activeCamera].updateCameraDirection(dx * sensitivity, dy * sensitivity);
    }
    double scrollDy = Mouse::getScrollDY();
    if (scrollDy != 0) {
        cameras[activeCamera].updateCameraZoom(scrollDy);
    }

    //Joystick for direction
    dx = mainJ.axesState(GLFW_JOYSTICK_AXES_RIGHT_STICK_X);
    dy = mainJ.axesState(GLFW_JOYSTICK_AXES_RIGHT_STICK_Y);
    if ((dx != 0) || (dy != 0)) {
        cameras[activeCamera].updateCameraDirection(dx * sensitivity, -dy * sensitivity);
    }

    //Joystick for camera position
    dx = mainJ.axesState(GLFW_JOYSTICK_AXES_LEFT_STICK_X);
    dy = mainJ.axesState(GLFW_JOYSTICK_AXES_LEFT_STICK_Y);
    if (dx != 0 || dy != 0) {
        cameras[activeCamera].updateCameraPosXbox(glm::vec3(dx, 0.0f, dy), dt);
    }

}