#include <DX3D/Graphics/Primitives/Cube.h>
#include <DX3D/Graphics/IndexBuffer.h>
#include <vector>

using namespace dx3d;
using namespace DirectX;

// Cube implementation
Cube::Cube() : AGameObject()
{
}

Cube::Cube(const Vector3& position, const Vector3& rotation, const Vector3& scale)
    : AGameObject(position, rotation, scale)
{
}

void Cube::update(float deltaTime)
{
    // Override this method to add cube-specific update logic if needed
    // Call base class update if needed
    AGameObject::update(deltaTime);
}

std::shared_ptr<VertexBuffer> Cube::CreateVertexBuffer(const GraphicsResourceDesc& resourceDesc)
{
    // Define cube vertices with positions and colors
    std::vector<Vertex> vertices;

    // Front face (red)
    vertices.push_back({ {-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.0f} });
    vertices.push_back({ {-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.0f} });
    vertices.push_back({ { 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.0f} });
    vertices.push_back({ { 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.0f} });

    // Back face (green)
    vertices.push_back({ {-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f, 1.0f} });
    vertices.push_back({ { 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f, 1.0f} });
    vertices.push_back({ { 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f, 1.0f} });
    vertices.push_back({ {-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f, 1.0f} });

    // Top face (blue)
    vertices.push_back({ {-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f, 1.0f} });
    vertices.push_back({ {-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f, 1.0f} });
    vertices.push_back({ { 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f, 1.0f} });
    vertices.push_back({ { 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f, 1.0f} });

    // Bottom face (yellow)
    vertices.push_back({ {-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f, 1.0f} });
    vertices.push_back({ { 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f, 1.0f} });
    vertices.push_back({ { 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 0.0f, 1.0f} });
    vertices.push_back({ {-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 0.0f, 1.0f} });

    // Right face (magenta)
    vertices.push_back({ { 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f, 1.0f} });
    vertices.push_back({ { 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 1.0f, 1.0f} });
    vertices.push_back({ { 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 1.0f, 1.0f} });
    vertices.push_back({ { 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 1.0f, 1.0f} });

    // Left face (cyan)
    vertices.push_back({ {-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f, 1.0f} });
    vertices.push_back({ {-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 1.0f, 1.0f} });
    vertices.push_back({ {-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 1.0f, 1.0f} });
    vertices.push_back({ {-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 1.0f, 1.0f} });

    return std::make_shared<VertexBuffer>(
        vertices.data(),
        sizeof(Vertex),
        static_cast<ui32>(vertices.size()),
        resourceDesc
    );
}

std::shared_ptr<IndexBuffer> Cube::CreateIndexBuffer(const GraphicsResourceDesc& resourceDesc)
{
    // Define cube indices (each face has 2 triangles)
    std::vector<ui32> indices = {
        // Front face
        0, 1, 2,    0, 2, 3,
        // Back face
        4, 5, 6,    4, 6, 7,
        // Top face
        8, 9, 10,   8, 10, 11,
        // Bottom face
        12, 13, 14, 12, 14, 15,
        // Right face
        16, 17, 18, 16, 18, 19,
        // Left face
        20, 21, 22, 20, 22, 23
    };

    // Create index buffer
    return std::make_shared<IndexBuffer>(
        indices.data(),
        static_cast<ui32>(indices.size()),
        resourceDesc
    );
}