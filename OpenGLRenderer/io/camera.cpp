#include "camera.h"
#include <iostream>

Camera::Camera(glm::vec3 position, NavigationType nt) :
        cameraPos(position),
        worldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
        yaw(0.0f),
        pitch(0.0f),
        speed(2.5f),
        zoom(45.0f),
        cameraFront(glm::vec3(1.0f, 0.0f, 0.0f)),
        navigationType(nt)
{
    updateCameraVectors();
}

void Camera::updateCameraDirection(double dx, double dy) {
    yaw += dx;
    pitch += dy;

    if (pitch > 89.0f) {
        pitch = 89.0f;
    }
    else if(pitch < -89.0f){
        pitch = -89.0f;
    }
    updateCameraVectors();

}
void Camera::updateCameraPos(CameraDirection direction, float dt) {
    float velocity = (float)dt * speed;
 
    glm::vec3 directionToMoveTo;
    if (navigationType == ALL_AXES) { // we can move wherever cameraFront is pointing to, even up or down
        directionToMoveTo = cameraFront;
    }
    else if(navigationType == X_AND_Z_ONLY) {
        directionToMoveTo = glm::vec3(cameraFront.x, 0.0, cameraFront.z);
        //normalize cause when the player cf.x & cf.y are both > 0, the directionToMoveTo vector becomes slightly more that unit
        directionToMoveTo = glm::normalize(directionToMoveTo); 
    }

    switch (direction) {
    case CameraDirection::FORWARD:
        cameraPos += directionToMoveTo * velocity;
        break;
    case CameraDirection::BACKWARD:
        cameraPos -= directionToMoveTo * velocity;
        break;
    case CameraDirection::RIGHT:
        cameraPos += cameraRight * velocity;
        break;
    case CameraDirection::LEFT:
        cameraPos -= cameraRight * velocity;
        break;
    case CameraDirection::UP:
        cameraPos += worldUp * velocity; //using world up insted of cameraup means when you jump you go up relative to the world, not relative to your direction
        break; 
    case CameraDirection::DOWN:
        cameraPos -= worldUp * velocity;
        break;
    }
}
void Camera::updateCameraPosXbox(glm::vec3 dir, float dt) {
    float velocity = (float)speed * dt;
    cameraPos += (glm::vec3(dir.x, dir.y, dir.z) * velocity);
}
void Camera::updateCameraZoom(float dy) {
    if (zoom >= 1.0f && zoom <= 45.0f) {
        zoom -= dy;
    }
    else if (zoom < 1.0f) {
        zoom = 1.0f;
    }
    else { //zoom > 45.0f
        zoom = 45.0f;
    }
}

float Camera::getZoom()
{
    return zoom;
}

glm::mat4x4 Camera::getViewMatrix() {
    return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

void Camera::switchNaviationType() {
    navigationType = (navigationType == ALL_AXES) ? X_AND_Z_ONLY : ALL_AXES;
}

glm::mat4x4 Camera::getViewMatrixNoPos() {
    return glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(2.0f, 2.0f, 2.0f) + cameraFront, cameraUp);
}

void Camera::updateCameraVectors() {
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    cameraFront = glm::normalize(direction);
    cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
}