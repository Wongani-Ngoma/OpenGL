#include "../model.h"
#include "../../../io/camera.h"

class Gun : public Model {
public:
    Gun() : 
        Model(glm::vec3(0.0), glm::vec3(0.5), true) {}

    void render(Shader shader, bool setModel = false) {
        glm::mat4 model = glm::mat4(1.0);

        // pos
        pos = Camera::defaultCamera.cameraPos +
            glm::vec3(Camera::defaultCamera.cameraFront * 8.0f) -
            glm::vec3(Camera::defaultCamera.cameraUp * -2.15f) +
            glm::vec3(Camera::defaultCamera.cameraRight * 2.15f);
        model = glm::translate(model, pos);

        //pitch rotation - rotate around cameraRight using dot product
        float theta = acos(glm::dot(Camera::defaultCamera.worldUp, Camera::defaultCamera.cameraFront) /
            glm::length(Camera::defaultCamera.cameraUp) / glm::length(Camera::defaultCamera.cameraFront));
        model = glm::rotate(model, atanf(1) * 2 - theta, Camera::defaultCamera.cameraRight); // offset by pi/2 radians because angle between cameraFront and gunFront

        // yaw rotation - rotate around cameraUp using dot product
        glm::vec2 front2D = glm::vec2(Camera::defaultCamera.cameraFront.x, Camera::defaultCamera.cameraFront.z);
        theta = acos(glm::dot(glm::vec2(1.0, 0.0), front2D) / glm::length(front2D));
        model = glm::rotate(model, (Camera::defaultCamera.cameraFront.z < 0) ? theta : -theta, Camera::defaultCamera.worldUp);

        // size
        model = glm::scale(model, size);
        
        shader.setMat4("model", model);

        Model::render(shader, false);

        gf = Camera::defaultCamera.cameraFront;
    }
    glm::vec2 gf;
};