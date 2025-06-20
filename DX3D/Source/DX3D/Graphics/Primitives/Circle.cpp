#include <DX3D/Graphics/Primitives/Circle.h>
#include <vector>
#include <cmath>

std::shared_ptr<dx3d::VertexBuffer> dx3d::Circle::Create(const GraphicsResourceDesc& resourceDesc, int segments, float aspectRatio)
{
    return CreateAt(resourceDesc, 0.0f, 0.0f, 0.5f, segments, aspectRatio);
}

std::shared_ptr<dx3d::VertexBuffer> dx3d::Circle::CreateAt(
    const GraphicsResourceDesc& resourceDesc,
    float centerX, float centerY,
    float radius,
    int segments,
    float aspectRatio)
{
    std::vector<Vertex> vertices;
    vertices.reserve(segments * 3);

    Vertex centerVertex = { {centerX, centerY, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f} };

    float angleIncrement = 2.0f * 3.14159265f / segments;

    float radiusX = radius / aspectRatio;

    for (int i = 0; i < segments; ++i)
    {
        float angle1 = i * angleIncrement;
        float angle2 = (i + 1) * angleIncrement;

        Vertex v1 = {
            {centerX + radiusX * std::cos(angle1), centerY + radius * std::sin(angle1), 0.0f},
            {1.0f, 1.0f, 0.0f, 1.0f}
        };

        Vertex v2 = {
            {centerX + radiusX * std::cos(angle2), centerY + radius * std::sin(angle2), 0.0f},
            {1.0f, 1.0f, 0.0f, 1.0f}
        };

        vertices.push_back(centerVertex);
        vertices.push_back(v1);
        vertices.push_back(v2);
    }

    return std::make_shared<VertexBuffer>(
        vertices.data(),
        sizeof(Vertex),
        static_cast<ui32>(vertices.size()),
        resourceDesc
    );
}