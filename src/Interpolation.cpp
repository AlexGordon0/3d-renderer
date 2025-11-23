#include "Interpolation.h"

std::vector<float> interpolateSingleFloats(float from, float to, int numberOfValues) {
    std::vector<float> interpolated = {};
    float step = (to - from) / (numberOfValues - 1);
    for (int i = 0; i < numberOfValues; i++) {
        interpolated.push_back(from + step * i);
    }
    return interpolated;
}

std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues) {
    std::vector<glm::vec3> interpolated = {};

    float step1 = (to.x - from.x) / (numberOfValues - 1);
    float step2 = (to.y - from.y) / (numberOfValues - 1);
    float step3 = (to.z - from.z) / (numberOfValues - 1);
    glm::vec3 steps = glm::vec3(step1, step2, step3);

    for (float i = 0; i < numberOfValues; i++) {
        interpolated.push_back(glm::vec3(from + steps * i));
    }
    return interpolated;
}


