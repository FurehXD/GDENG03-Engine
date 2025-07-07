#pragma once
#include <DX3D/Graphics/Primitives/AGameObject.h>
#include <DX3D/Graphics/VertexBuffer.h>
#include <DX3D/Graphics/IndexBuffer.h>
#include <DX3D/Math/Math.h>
#include <memory>

#define SEGMENTS (ui32)12

namespace dx3d
{
    /*class VertexBuffer;
    class IndexBuffer;
    class GraphicsResourceDesc;*/

    class CameraGizmo
    {
    public:
        // Static methods for creating rendering resources
        static std::shared_ptr<VertexBuffer> CreateVertexBuffer(const GraphicsResourceDesc& resourceDesc);
        static std::shared_ptr<IndexBuffer> CreateIndexBuffer(const GraphicsResourceDesc& resourceDesc);

        // Get the number of indices for a camera gizmo
        static ui32 GetIndexCount() { return SEGMENTS * SEGMENTS * 3; }
    };
}