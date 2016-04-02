#include <scene/geometry/geometry.h>

float Geometry::RayPDF(const Intersection &isx, const Ray &ray)
{
    //The isx passed in was tested ONLY against us (no other scene objects), so we test if NULL
    //rather than if != this.
    if(isx.object_hit == NULL)
    {
        return 0;
    }

    //Add more here
    Intersection isxOnGeometry = this->GetIntersection(ray);
    float r = glm::distance(isx.point, isxOnGeometry.point);
    float cosTheta = glm::dot(-ray.direction, isxOnGeometry.normal);

    float pdf = r * r / cosTheta / area;
    return pdf;
}
