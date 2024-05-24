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
#include "graphics/models/gun.hpp"
#include "graphics/models/axesOverlay.hpp"

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

Camera Camera::defaultCamera(glm::vec3(0.0f));

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

    Model container(glm::vec3(0.0, -1.0, 0.0f), glm::vec3(1.0), true);
    container.loadModel("assets\\models\\container\\source\\container.obj");

    Gun gun;
    gun.loadModel("assets\\models\\low-poly-ak-47.obj");

    AxesOverlay ao;
    ao.loadModel("assets\\models\\axis.obj");

    Model sphere(glm::vec3(0.0, 6.0, 0.0), glm::vec3(1.0), true);
    sphere.loadModel("assets\\models\\sphere.obj");

    DirLight dirLight = { glm::vec3(-0.2, -1.0, -0.3),
        glm::vec4(0.1, 0.1, 0.1, 1.0),
        glm::vec4(0.4, 0.4, 0.4, 1.0f), 
        glm::vec4(0.75, 0.75, 0.75, 1.0f) 
    };

    glm::vec3 pointLightPositions[] = {
            glm::vec3(2.3f, 5.3f, -4.0f),
            glm::vec3(-4.0f,  2.0f, 12.0f),
    };

    Lamp lamps[2];
    for (int i = 0; i < 2; i++) {
        lamps[i] = Lamp( 
            glm::vec3(50.0f, 50.0f, 130.0f),
            glm::vec4(2.0, 2.0, 2.0, 2.0),
            glm::vec4(2.0, 2.0, 2.0, 2.0),
            glm::vec4(2.0, 2.0, 2.0, 2.0),
            1.0, 0.07, 0.032,
            pointLightPositions[i],
            glm::vec3(1.0)
        );
        lamps[i].material = Material::turquoise;
        lamps[i].init();
    }

    SpotLight spotLight = {
        Camera::defaultCamera.cameraPos, Camera::defaultCamera.cameraFront,
        glm::cos(glm::radians(12.5)), glm::cos(glm::radians(20.0)),
        1.0f, 0.07f, 0.032f,
        glm::vec4(2.0, 2.0, 2.0, 1.0f), glm::vec4(2.0, 2.0, 2.0, 1.0), glm::vec4(2.0, 2.0, 2.0, 1.0)
    };

    mainJ.update();
    if (mainJ.isPresent()) {
        std::cout << mainJ.getName() << " is present.\n";
    }
    else {
        std::cout << "No controller present.\n";
    }

    glViewport(0, 0, screen.SCR_WIDTH, screen.SCR_HEIGHT);
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
        shader.set3Float("viewPos", Camera::defaultCamera.cameraPos);
        
        /*
        * rotating the directional light like the sun travelling through the sky
        dirLight.direction = glm::vec3(
            glm::rotate(glm::mat4(1.0f), (float)glm::radians(0.5f), glm::vec3(1.0f, 0.0f, 0.0f)) *
            glm::vec4(dirLight.direction, 1.0f));
        dirLight.render(shader);
        */

        shader.setInt("noPointLights", 2);
        for (int i = 0; i < 2; i++) {
            lamps[i].pointLight.render(shader, i);
        }
    
        if (flashlightOn) {
            spotLight.position = Camera::defaultCamera.cameraPos;
            spotLight.direction = Camera::defaultCamera.cameraFront;
            spotLight.render(shader, 0);
            shader.setInt("noSpotLights", 1);
        }
        else {
            shader.setInt("noSpotLights", 0);
        }


        //Create transformation for screen
        glm::mat4 view = glm::mat4(1.0f); // from world to camera, specific to each camera
        glm::mat4 projection = glm::mat4(1.0); // general, actually specific to each viewport
        view = Camera::defaultCamera.getViewMatrix();
        projection = glm::perspective(glm::radians(Camera::defaultCamera.getZoom()), (float)screen.SCR_WIDTH / (float)screen.SCR_HEIGHT, 0.1f, 100.0f);

        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        for (int i = 0; i < 2; i++) {
            lamps[i].render(shader);
        }

        container.render(shader);
        gun.render(shader);
        ao.render(shader);

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
    gun.cleanup();
    ao.cleanup();

    for (int i = 0; i < 2; i++) {
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

    // move camera
    if (Keyboard::key(GLFW_KEY_UP)) {
        Camera::defaultCamera.updateCameraPos(CameraDirection::FORWARD, dt);
    }
    if (Keyboard::key(GLFW_KEY_DOWN)) {
        Camera::defaultCamera.updateCameraPos(CameraDirection::BACKWARD, dt);
    }
    if (Keyboard::key(GLFW_KEY_LEFT)) {
        Camera::defaultCamera.updateCameraPos(CameraDirection::LEFT, dt);
    }
    if (Keyboard::key(GLFW_KEY_RIGHT)) {
        Camera::defaultCamera.updateCameraPos(CameraDirection::RIGHT, dt);
    }
    if (Keyboard::key(GLFW_KEY_SPACE)) {
        Camera::defaultCamera.updateCameraPos(CameraDirection::UP, dt);
    }
    if (Keyboard::key(GLFW_KEY_LEFT_SHIFT)) {
        Camera::defaultCamera.updateCameraPos(CameraDirection::DOWN, dt);
    }

    // select navigation type 
    if (Keyboard::keyWentDown(GLFW_KEY_CAPS_LOCK)) {
        Camera::defaultCamera.switchNaviationType();
    }

    // toogle spotlight on/off
    if (Keyboard::keyWentDown(GLFW_KEY_F)) {
        flashlightOn = !flashlightOn;
    }

    //keyboard for direction
    if (Keyboard::key(GLFW_KEY_W)) {
        Camera::defaultCamera.updateCameraDirection(0.0, 0.25);
    }
    if (Keyboard::key(GLFW_KEY_S)) {
        Camera::defaultCamera.updateCameraDirection(0.0, -0.25);
    }
    if (Keyboard::key(GLFW_KEY_A)) {
        Camera::defaultCamera.updateCameraDirection(-0.25, 0.0);
    }
    if (Keyboard::key(GLFW_KEY_D)) {
        Camera::defaultCamera.updateCameraDirection(0.25, 0.0);
    }

    //Mouse for direction
    double dx = Mouse::getDX(), dy = Mouse::getDY();
    if (dx != 0 || dy != 0) {
        Camera::defaultCamera.updateCameraDirection(dx * sensitivity, dy * sensitivity);
    }
    double scrollDy = Mouse::getScrollDY();
    if (scrollDy != 0) {
        Camera::defaultCamera.updateCameraZoom(scrollDy);
    }

    //Joystick for direction
    dx = mainJ.axesState(GLFW_JOYSTICK_AXES_RIGHT_STICK_X);
    dy = mainJ.axesState(GLFW_JOYSTICK_AXES_RIGHT_STICK_Y);
    if ((dx != 0) || (dy != 0)) {
        Camera::defaultCamera.updateCameraDirection(dx * sensitivity, -dy * sensitivity);
    }

    //Joystick for camera position
    dx = mainJ.axesState(GLFW_JOYSTICK_AXES_LEFT_STICK_X);
    dy = mainJ.axesState(GLFW_JOYSTICK_AXES_LEFT_STICK_Y);
    if (dx != 0 || dy != 0) {
        Camera::defaultCamera.updateCameraPosXbox(glm::vec3(dx, 0.0f, dy), dt);
    }

}