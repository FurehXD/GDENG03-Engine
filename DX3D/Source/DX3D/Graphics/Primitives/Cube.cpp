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
    std::cout << this->getObjectType() << std::endl;
}

void Cube::update(float deltaTime)
{
    AGameObject::update(deltaTime);
}

std::shared_ptr<VertexBuffer> Cube::CreateVertexBuffer(const GraphicsResourceDesc& resourceDesc)
{
    std::vector<Vertex> vertices = {
        // Front face (Correct)
        Vertex({-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}),
        Vertex({-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}),
        Vertex({ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}),
        Vertex({ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}),

        // Back face (Correct)
        Vertex({-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}),
        Vertex({-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}),
        Vertex({ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}),
        Vertex({ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}),

        // Top face (Correct)
        Vertex({-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}),
        Vertex({-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}),
        Vertex({ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}),
        Vertex({ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}),

        // Bottom face (VERTEX ORDER CORRECTED)
        Vertex({-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}),
        Vertex({-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}),
        Vertex({ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}),
        Vertex({ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}),

        // Right face (Correct)
        Vertex({ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}),
        Vertex({ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}),
        Vertex({ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}),
        Vertex({ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}),

        // Left face (Correct)
        Vertex({-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}),
        Vertex({-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}),
        Vertex({-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}),
        Vertex({-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f})
    };

    return std::make_shared<VertexBuffer>(
        vertices.data(),
        sizeof(Vertex),
        static_cast<ui32>(vertices.size()),
        resourceDesc
    );
}
std::shared_ptr<IndexBuffer> Cube::CreateIndexBuffer(const GraphicsResourceDesc& resourceDesc)
{
    // Winding order for all faces is now corrected to be counter-clockwise (CCW).
    std::vector<ui32> indices = {
        // Front face (Correct)
        0, 1, 2,    0, 2, 3,

        // Back face (Corrected)
        7, 6, 5,    7, 5, 4,

        // Top face (Correct)
        8, 9, 10,   8, 10, 11,

        // Bottom face (Corrected)
        15, 14, 13, 15, 13, 12,

        // Right face (Correct)
        16, 17, 18, 16, 18, 19,

        // Left face (Corrected)
        23, 22, 21, 23, 21, 20
    };

    return std::make_shared<IndexBuffer>(
        indices.data(),
        static_cast<ui32>(indices.size()),
        resourceDesc
    );
}