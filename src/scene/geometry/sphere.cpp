#include "sphere.h"

#include <iostream>

#include <la.h>
#include <math.h>

static const int SPH_IDX_COUNT = 2280;  // 760 tris * 3
static const int SPH_VERT_COUNT = 382;

void Sphere::ComputeArea()
{
    //Extra credit to implement this  
    float detT = glm::determinant(transform.T());
    float r = 0.5f;
    float originalArea = 4.0f * PI * r * r;
    area = originalArea * detT;
}

glm::vec4 GetSphereTangent(const glm::vec3 &normal)
{
    glm::vec3 up(0,1,0);
    return glm::vec4(glm::normalize(glm::cross(up, normal)),0);
}

glm::vec4 GetSphereBitangent(const glm::vec3 &normal, const glm::vec3 &tangent)
{
    return glm::vec4(glm::normalize(glm::cross(normal, tangent)),0);
}

Intersection Sphere::SampleOnGeometrySurface(const float &u, const float &v, const glm::vec3 &point)
{
    glm::vec3 direction(transform.position() - point);
    Ray r(point, direction);
    Intersection intersection = this->GetIntersection(r);
    if(intersection.t > 0)
    {
        // angle change is between [-PI/2,PI/2], guarante the point will the same side of intersection after sample
        float du = u * PI ;
        float dv = v  * 2.0f * PI ;

        glm::vec3 localPoint = glm::vec3(transform.invT() * glm::vec4(intersection.point, 1.0f));
        float r = 0.5f;
        float theta = asin(localPoint.y / r);
        float phi = atan2(localPoint.z , localPoint.x);

        theta += du;
        phi += dv;
        glm::vec3 localSamplePoint(r*cos(theta)*cos(phi),
                                   r*sin(theta),
                                   r*cos(theta)*sin(phi));

        Intersection sampleResult;
        sampleResult.point = glm::vec3(transform.T() * glm::vec4(localSamplePoint, 1.0f));
        sampleResult.normal = glm::normalize(glm::vec3(transform.invTransT() * glm::vec4(glm::normalize(localPoint),0.0f)));
        sampleResult.tangent = glm::normalize(glm::vec3(transform.invTransT() * GetSphereTangent(sampleResult.normal)));
        sampleResult.bitangent = glm::normalize(glm::vec3(transform.invTransT() * GetSphereBitangent(sampleResult.normal,sampleResult.tangent)));
        sampleResult.t = glm::length(sampleResult.point - point);
        sampleResult.object_hit = this;
        sampleResult.texture_color = Material::GetImageColorInterp(GetUVCoordinates(localSamplePoint), material->texture);//useless for sampling
        return sampleResult;
    }
    else
        return Intersection();
}

glm::vec3 Sphere::ComputeNormal(const glm::vec3 &P)
{}

Intersection Sphere::GetIntersection(Ray r)
{
    //Transform the ray
    Ray r_loc = r.GetTransformedCopy(transform.invT());
    Intersection result;

    float A = pow(r_loc.direction[0], 2) + pow(r_loc.direction[1], 2) + pow(r_loc.direction[2], 2);
    float B = 2*(r_loc.direction[0]*r_loc.origin[0] + r_loc.direction[1] * r_loc.origin[1] + r_loc.direction[2] * r_loc.origin[2]);
    float C = pow(r_loc.origin[0], 2) + pow(r_loc.origin[1], 2) + pow(r_loc.origin[2], 2) - 0.25f;//Radius is 0.5f
    float discriminant = B*B - 4*A*C;
    //If the discriminant is negative, then there is no real root
    if(discriminant < 0){
        return result;
    }
    float t = (-B - sqrt(discriminant))/(2*A);
    if(t < 0)
    {
        t = (-B + sqrt(discriminant))/(2*A);
    }
    if(t >= 0)
    {
        glm::vec4 P = glm::vec4(r_loc.origin + t*r_loc.direction, 1);
        result.point = glm::vec3(transform.T() * P);
        glm::vec3 normal = glm::normalize(glm::vec3(P));
        glm::vec2 uv = GetUVCoordinates(glm::vec3(P));
        result.normal = glm::normalize(glm::vec3(transform.invTransT() * (P - glm::vec4(0,0,0,1))));
        result.t = glm::distance(result.point, r.origin);
        result.texture_color = Material::GetImageColor(uv, material->texture);
        result.object_hit = this;

        // Store the tangent and bitangent
        result.tangent = glm::normalize(glm::vec3(transform.invTransT() * GetSphereTangent(result.normal)));
        result.bitangent = glm::normalize(glm::vec3(transform.invTransT() * GetSphereBitangent(result.normal, result.tangent)));
        return result;
    }
    return result;
}

glm::vec2 Sphere::GetUVCoordinates(const glm::vec3 &point)
{
//    glm::vec3 p = glm::normalize(point);
//    float phi = atan2f(p.z, p.x);//glm::atan(p.x/p.z);
//    if(phi < 0)
//    {
//        phi += TWO_PI;
//    }
//    float theta = glm::acos(p.y);
//    return glm::vec2(1 - phi/TWO_PI, 1 - theta / PI);

    glm::vec3 d = glm::normalize(point);
    float u = 0.5f + atan2(d.x, d.z) / (2.0f * PI);
    float v = asin(d.y) / PI + 0.5f;
    return glm::vec2(u, v);
}

// These are functions that are only defined in this cpp file. They're used for organizational purposes
// when filling the arrays used to hold the vertex and index data.
void createSphereVertexPositions(glm::vec3 (&sph_vert_pos)[SPH_VERT_COUNT])
{
    // Create rings of vertices for the non-pole vertices
    // These will fill indices 1 - 380. Indices 0 and 381 will be filled by the two pole vertices.
    glm::vec4 v;
    // i is the Z axis rotation
    for (int i = 1; i < 19; i++) {
        // j is the Y axis rotation
        for (int j = 0; j < 20; j++) {
            v = glm::rotate(glm::mat4(1.0f), j / 20.f * TWO_PI, glm::vec3(0, 1, 0))
                * glm::rotate(glm::mat4(1.0f), -i / 18.0f * PI, glm::vec3(0, 0, 1))
                * glm::vec4(0, 0.5f, 0, 1);
            sph_vert_pos[(i - 1) * 20 + j + 1] = glm::vec3(v);
        }
    }
    // Add the pole vertices
    sph_vert_pos[0] = glm::vec3(0, 0.5f, 0);
    sph_vert_pos[381] = glm::vec3(0, -0.5f, 0);  // 361 - 380 are the vertices for the bottom cap
}


void createSphereVertexNormals(glm::vec3 (&sph_vert_nor)[SPH_VERT_COUNT])
{
    // Unlike a cylinder, a sphere only needs to be one normal per vertex
    // because a sphere does not have sharp edges.
    glm::vec4 v;
    // i is the Z axis rotation
    for (int i = 1; i < 19; i++) {
        // j is the Y axis rotation
        for (int j = 0; j < 20; j++) {
            v = glm::rotate(glm::mat4(1.0f), j / 20.0f * TWO_PI, glm::vec3(0, 1, 0))
                * glm::rotate(glm::mat4(1.0f), -i / 18.0f * PI, glm::vec3(0, 0, 1))
                * glm::vec4(0, 1.0f, 0, 0);
            sph_vert_nor[(i - 1) * 20 + j + 1] = glm::vec3(v);
        }
    }
    // Add the pole normals
    sph_vert_nor[0] = glm::vec3(0, 1.0f, 0);
    sph_vert_nor[381] = glm::vec3(0, -1.0f, 0);
}


void createSphereIndices(GLuint (&sph_idx)[SPH_IDX_COUNT])
{
    int index = 0;
    // Build indices for the top cap (20 tris, indices 0 - 60, up to vertex 20)
    for (int i = 0; i < 19; i++) {
        sph_idx[index] = 0;
        sph_idx[index + 1] = i + 1;
        sph_idx[index + 2] = i + 2;
        index += 3;
    }
    // Must create the last triangle separately because its indices loop
    sph_idx[57] = 0;
    sph_idx[58] = 20;
    sph_idx[59] = 1;
    index += 3;

    // Build indices for the body vertices
    // i is the Z axis rotation
    for (int i = 1; i < 19; i++) {
        // j is the Y axis rotation
        for (int j = 0; j < 20; j++) {
            sph_idx[index] = (i - 1) * 20 + j + 1;
            sph_idx[index + 1] = (i - 1) * 20 +  j + 2;
            sph_idx[index + 2] = (i - 1) * 20 +  j + 22;
            sph_idx[index + 3] = (i - 1) * 20 +  j + 1;
            sph_idx[index + 4] = (i - 1) * 20 +  j + 22;
            sph_idx[index + 5] = (i - 1) * 20 +  j + 21;
            index += 6;
        }
    }

    // Build indices for the bottom cap (20 tris, indices 2220 - 2279)
    for (int i = 0; i < 19; i++) {
        sph_idx[index] = 381;
        sph_idx[index + 1] = i + 361;
        sph_idx[index + 2] = i + 362;
        index += 3;
    }
    // Must create the last triangle separately because its indices loop
    sph_idx[2277] = 381;
    sph_idx[2278] = 380;
    sph_idx[2279] = 361;
    index += 3;
}

void Sphere::create()
{
    GLuint sph_idx[SPH_IDX_COUNT];
    glm::vec3 sph_vert_pos[SPH_VERT_COUNT];
    glm::vec3 sph_vert_nor[SPH_VERT_COUNT];
    glm::vec3 sph_vert_col[SPH_VERT_COUNT];

    createSphereVertexPositions(sph_vert_pos);
    createSphereVertexNormals(sph_vert_nor);
    createSphereIndices(sph_idx);
    for (int i = 0; i < SPH_VERT_COUNT; i++) {
        sph_vert_col[i] = material->base_color;
    }

    count = SPH_IDX_COUNT;

    bufIdx.create();
    bufIdx.bind();
    bufIdx.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufIdx.allocate(sph_idx, SPH_IDX_COUNT * sizeof(GLuint));

    bufPos.create();
    bufPos.bind();
    bufPos.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufPos.allocate(sph_vert_pos, SPH_VERT_COUNT * sizeof(glm::vec3));

    bufCol.create();
    bufCol.bind();
    bufCol.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufCol.allocate(sph_vert_col, SPH_VERT_COUNT * sizeof(glm::vec3));

    bufNor.create();
    bufNor.bind();
    bufNor.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufNor.allocate(sph_vert_nor, SPH_VERT_COUNT * sizeof(glm::vec3));
}
