#include "../model.h"
#include "../../../io/camera.h"

class AxesOverlay : public Model {
public:
    AxesOverlay() : 
        Model(glm::vec3(Camera::defaultCamera.cameraPos * 1.1f), glm::vec3(0.05), true)
    {}
    void render(Shader shader, bool setModel = false) {

        glm::mat4 model(1.0);
        model = glm::translate(model, Camera::defaultCamera.cameraPos + 
            glm::vec3(Camera::defaultCamera.cameraUp * 0.30f) +
            glm::vec3(Camera::defaultCamera.cameraRight * 0.60f) +
            glm::vec3(Camera::defaultCamera.cameraFront));

        model = glm::scale(model, size);

        shader.setMat4("model", model);
        Model::render(shader, false);
    }
};