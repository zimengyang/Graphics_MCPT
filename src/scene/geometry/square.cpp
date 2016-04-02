#include <scene/geometry/square.h>

void SquarePlane::ComputeArea()
{
    // use linear transformation matrix to calculate the area of shape
    float detT = glm::determinant(transform.T());
    float r = 1.0f;
    float originArea = r*r;

    area = detT*originArea;
    if(transform.getScale().z != 1.0f)
        area = area/transform.getScale().z;
}

glm::vec4 GetSquareTangent(const glm::vec3& normal)
{
    glm::vec3 up(0,1,0);
    return glm::vec4(glm::normalize(glm::cross(up,normal)),0);
}

glm::vec4 GetSquareBitangent(const glm::vec3& normal, const glm::vec3& tangent)
{
    return glm::vec4(glm::normalize(glm::cross(normal, tangent)),0);
}

Intersection SquarePlane::SampleOnGeometrySurface(const float &u, const float &v, const glm::vec3 &point)
{
    glm::vec3 hitPoint(u - 0.5f, v - 0.5f, 0.0f);
    Intersection result;
    result.point = glm::vec3(transform.T() * glm::vec4(hitPoint,1.0f));
    result.normal = glm::normalize(glm::vec3(transform.invTransT() * glm::vec4(ComputeNormal(hitPoint), 0)));
    result.object_hit = this;
    result.texture_color = Material::GetImageColorInterp(GetUVCoordinates(hitPoint), material->texture);

    result.tangent = glm::normalize(glm::vec3(transform.invTransT() * GetSquareTangent(result.normal)));
    result.bitangent = glm::normalize(glm::vec3(transform.invTransT() * GetSquareBitangent(result.normal,result.tangent)));
    // what's the purpose of t of a sample intersection on light surface?
    result.t = 1.0f;

    return result;
}

Intersection SquarePlane::GetIntersection(Ray r)
{
    //Transform the ray
    Ray r_loc = r.GetTransformedCopy(transform.invT());
    Intersection result;

    //Ray-plane intersection
    float t = glm::dot(glm::vec3(0,0,1), (glm::vec3(0.5f, 0.5f, 0) - r_loc.origin)) / glm::dot(glm::vec3(0,0,1), r_loc.direction);
    glm::vec4 P = glm::vec4(t * r_loc.direction + r_loc.origin, 1);
    //Check that P is within the bounds of the square
    if(t > 0 && P.x >= -0.5f && P.x <= 0.5f && P.y >= -0.5f && P.y <= 0.5f)
    {
        result.point = glm::vec3(transform.T() * P);
        result.normal = glm::normalize(glm::vec3(transform.invTransT() * glm::vec4(ComputeNormal(glm::vec3(P)), 0)));
        result.object_hit = this;
        result.t = glm::distance(result.point, r.origin);
        result.texture_color = Material::GetImageColorInterp(GetUVCoordinates(glm::vec3(P)), material->texture);

        //Store the tangent and bitangent
        result.tangent = glm::normalize(glm::vec3(transform.invTransT() * GetSquareTangent(result.normal)));
        result.bitangent = glm::normalize(glm::vec3(transform.invTransT() * GetSquareBitangent(result.normal,result.tangent)));
        return result;
    }
    return result;
}


glm::vec2 SquarePlane::GetUVCoordinates(const glm::vec3 &point)
{
    return glm::vec2(point.x + 0.5f, point.y + 0.5f);
}

glm::vec3 SquarePlane::ComputeNormal(const glm::vec3 &P)
{
        return glm::vec3(0,0,1);
}

void SquarePlane::create()
{
    GLuint cub_idx[6];
    glm::vec3 cub_vert_pos[4];
    glm::vec3 cub_vert_nor[4];
    glm::vec3 cub_vert_col[4];

    cub_vert_pos[0] = glm::vec3(-0.5f, 0.5f, 0);  cub_vert_nor[0] = glm::vec3(0, 0, 1); cub_vert_col[0] = material->base_color;
    cub_vert_pos[1] = glm::vec3(-0.5f, -0.5f, 0); cub_vert_nor[1] = glm::vec3(0, 0, 1); cub_vert_col[1] = material->base_color;
    cub_vert_pos[2] = glm::vec3(0.5f, -0.5f, 0);  cub_vert_nor[2] = glm::vec3(0, 0, 1); cub_vert_col[2] = material->base_color;
    cub_vert_pos[3] = glm::vec3(0.5f, 0.5f, 0);   cub_vert_nor[3] = glm::vec3(0, 0, 1); cub_vert_col[3] = material->base_color;

    cub_idx[0] = 0; cub_idx[1] = 1; cub_idx[2] = 2;
    cub_idx[3] = 0; cub_idx[4] = 2; cub_idx[5] = 3;

    count = 6;

    bufIdx.create();
    bufIdx.bind();
    bufIdx.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufIdx.allocate(cub_idx, 6 * sizeof(GLuint));

    bufPos.create();
    bufPos.bind();
    bufPos.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufPos.allocate(cub_vert_pos, 4 * sizeof(glm::vec3));

    bufNor.create();
    bufNor.bind();
    bufNor.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufNor.allocate(cub_vert_nor, 4 * sizeof(glm::vec3));

    bufCol.create();
    bufCol.bind();
    bufCol.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bufCol.allocate(cub_vert_col, 4 * sizeof(glm::vec3));
}
