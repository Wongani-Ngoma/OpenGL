#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class CameraDirection {
    NONE = 0,
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera {
public:
    glm::vec3 cameraPos;

    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    glm::vec3 cameraRight;

    glm::vec3 worldUp;

    float yaw;
    float pitch;
    float speed;
    float zoom;

    Camera(glm::vec3 position);
    void updateCameraDirection(double dx, double dy);
    void updateCameraPos(CameraDirection dir, float dt);
    void updateCameraPosXbox(glm::vec3 dir, float dy); //The implementation of this one's different from the other one
    void updateCameraZoom(float dt);

    float getZoom();

    glm::mat4x4 getViewMatrixNoPos();
    glm::mat4x4 getViewMatrix();

private:
    void updateCameraVectors();
};

#endif

