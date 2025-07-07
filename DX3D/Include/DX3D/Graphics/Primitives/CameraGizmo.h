#pragma once
#include <DX3D/Graphics/Primitives/AGameObject.h>
#include <DX3D/Graphics/VertexBuffer.h>
#include <DX3D/Graphics/IndexBuffer.h>
#include <DX3D/Math/Math.h>
#include <memory>

#define SEGMENTS_VALUE 1
// If SEGMENTS_VALUE is less than 2, it will be clamped to 2.
#define SEGMENTS ((SEGMENTS_VALUE < 2) ? 2 : SEGMENTS_VALUE)

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
        // 36 because there are 3 arrows ((6 indices for a square for the cylinder + 6 indices for a square for the cone) *3)
        static ui32 GetIndexCount() { return SEGMENTS * 36; }
    };
}