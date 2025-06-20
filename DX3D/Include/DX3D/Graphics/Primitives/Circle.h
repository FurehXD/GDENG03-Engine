#pragma once
#include <DX3D/Graphics/VertexBuffer.h>
#include <memory>

namespace dx3d
{
    class Circle
    {
    public:
        static std::shared_ptr<VertexBuffer> Create(const GraphicsResourceDesc& resourceDesc, int segments = 32, float aspectRatio = 1.0f);

        static std::shared_ptr<VertexBuffer> CreateAt(
            const GraphicsResourceDesc& resourceDesc,
            float centerX, float centerY,
            float radius = 0.5f,
            int segments = 32,
            float aspectRatio = 1.0f
        );
    };
}