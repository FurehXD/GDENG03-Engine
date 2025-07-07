#include <DX3D/Graphics/Primitives/CameraIcon.h>
#include <DX3D/Graphics/Vertex.h>
#include <vector>

using namespace dx3d;

CameraIcon::CameraIcon(const Vector3& position, const Vector3& rotation, const Vector3& scale)
    : AGameObject(position, rotation, scale)
{
}

std::shared_ptr<VertexBuffer> CameraIcon::CreateVertexBuffer(const GraphicsResourceDesc& resourceDesc)
{
    // Texcoords: 'u' is in color.r, 'v' is in color.g (from rgba)
    const std::vector<Vertex> vertices = {
        // Position                 // Color (Tex Coords: u, v, 0, 0)
        { {-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f} }, // 0: Bottom Left
        { {-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f} }, // 1: Top Left
        { { 0.5f,  0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f} }, // 2: Top Right
        { { 0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 0.0f, 0.0f} }  // 3: Bottom Right
    };

    return std::make_shared<VertexBuffer>(
        vertices.data(),
        sizeof(Vertex),
        static_cast<ui32>(vertices.size()),
        resourceDesc
    );
}

std::shared_ptr<IndexBuffer> CameraIcon::CreateIndexBuffer(const GraphicsResourceDesc& resourceDesc)
{
    
    const std::vector<ui32> indices = { 0, 1, 2,  0, 2, 3 };

    return std::make_shared<IndexBuffer>(
        indices.data(),
        static_cast<ui32>(indices.size()),
        resourceDesc
    );
}