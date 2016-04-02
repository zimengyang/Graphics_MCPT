#include <scene/materials/bxdfs/phongbxdf.h>

glm::vec3 PhongBxDF::EvaluateScatteredEnergy(const glm::vec3 &wo, const glm::vec3 &wi) const
{
    float p = glm::dot(wo,wi);
    if(p < 0.0f)
        p = 0.0f;
    return (diffuse_color + glm::pow(p, specular_power) * specular_color)/ PI;
}

glm::vec3 PhongBxDF::EvaluateHemisphereScatteredEnergy(const glm::vec3 &wo, int num_samples, const glm::vec2 *samples) const
{
    //TODO
    return glm::vec3(0);
}
