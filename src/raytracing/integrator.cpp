#include <raytracing/integrator.h>


Integrator::Integrator():
    max_depth(5),
    MC_ESTIMATOR(10)
{
    scene = NULL;
    intersection_engine = NULL;
}

glm::vec3 ComponentMult(const glm::vec3 &a, const glm::vec3 &b)
{
    return glm::vec3(a.x * b.x, a.y * b.y, a.z * b.z);
}

void Integrator::SetDepth(unsigned int depth)
{
    max_depth = depth;
}

//Basic ray trace
glm::vec3 Integrator::TraceRay(Ray r, unsigned int depth)
{
    //TODO
    return glm::vec3(0,0,0);
}

DirectLightingIntegrator::DirectLightingIntegrator():
    Integrator()
{
}


//direct lighting integrator
glm::vec3 DirectLightingIntegrator::TraceRay(Ray r, unsigned int depth)
{
    if(depth >= max_depth)
        return glm::vec3(0);

    Intersection intersection = intersection_engine->GetIntersection(r);
    if(intersection.t > 0)
    {
        if(intersection.object_hit->material->is_light_source)
            return intersection.object_hit->material->base_color * intersection.texture_color;

        glm::vec3 resultColor(0);

        //random number generator and uniform distribution
        std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());
        std::uniform_real_distribution<float> uniform_distribution(0.0f,1.0f);

        for(Geometry* light : scene->lights)
        {
            // solve for MC estimator equation
            glm::vec3 sum(0);
            for(int i = 0; i < MC_ESTIMATOR;i++)
            {
                float u = uniform_distribution(generator);
                float v = uniform_distribution(generator);

                Intersection lightSample = light->SampleOnGeometrySurface(u, v, intersection.point + float(1e-3)*intersection.normal);
                glm::vec3 wj = glm::normalize(lightSample.point - intersection.point);
                glm::vec3 wo = - r.direction;
                glm::vec3 P = intersection.point;
                glm::vec3 N = intersection.normal;
                Intersection lightIntersection = intersection_engine->GetIntersection(Ray(P + float(1e-3)*N, wj));
                float pdf_wj = light->RayPDF(intersection, Ray(P + float(1e-3)*N, wj));

                // reach light directly && pdf(wj) > 0
                if(lightIntersection.t > 0 && lightIntersection.object_hit == light && pdf_wj > 0 )
                {
                    sum = sum + intersection.object_hit->material->EvaluateScatteredEnergy(intersection, glm::reflect(-wo,intersection.normal), wj) *
                                light->material->EvaluateScatteredEnergy(lightSample, -wj, -wj) *
                                float(fabs(glm::dot(wj, N))) / pdf_wj;
                }
            }
            resultColor = resultColor + sum / float(MC_ESTIMATOR);

        }
        resultColor = resultColor / float(scene->lights.size());
        return resultColor;
    }
    else
        return glm::vec3(0);

}
