#include <DX3D/Graphics/Primitives/CameraGizmo.h>
#include <DX3D/Graphics/VertexBuffer.h>
#include <DX3D/Graphics/IndexBuffer.h>
#include <DX3D/Graphics/Vertex.h>
#include <vector>

using namespace dx3d;

// Helper function to create the vertices for a single arrow
void CreateArrowVertices(std::vector<Vertex>& vertices, const Vector3& direction, const Vector4& color)
{
    const float cylinderLength = 0.8f;
    const float cylinderRadius = 0.02f;
    const float coneLength = 0.2f;
    const float codeRadius = 0.08f;
    const ui32 segments = SEGMENTS;

    Vector3 cylinderEnd = direction * cylinderLength;
    Vector3 coneBase = cylinderEnd;
    Vector3 coneTip = direction * (cylinderLength + coneLength);

    // Create orthonormal basis vectors for the arrow's orientation
    Vector3 up = (abs(direction.y) < 0.9f) ? Vector3(0, 1, 0) : Vector3(1, 0, 0);

    Vector3 right = Vector3::cross(up, direction);

    up = Vector3::cross(direction, right);

    // cylinder Vertices
    for (ui32 i = 0; i <= segments; ++i)
    {
        float angle = i * 2.0f * 3.14159265f / segments;
        Vector3 offset = (right * cos(angle) + up * sin(angle)) * cylinderRadius;

        vertices.push_back({ {offset.x, offset.y, offset.z}, {color.x, color.y, color.z, color.w} });
        Vector3 cylinderTipPosition = cylinderEnd + offset;
        vertices.push_back({ {cylinderTipPosition.x, cylinderTipPosition.y, cylinderTipPosition.z}, {color.x, color.y, color.z, color.w} });
    }

    // cone Vertices
    vertices.push_back({ {coneBase.x, coneBase.y, coneBase.z}, {color.x, color.y, color.z, color.w} }); // Center of base
    for (ui32 i = 0; i <= segments; ++i)
    {
        float angle = i * 2.0f * 3.14159265f / segments;
        Vector3 offset = (right * cos(angle) + up * sin(angle)) * codeRadius;

        Vector3 coneBasePosition = coneBase + offset;
        vertices.push_back({ {coneBasePosition.x, coneBasePosition.y, coneBasePosition.z}, {color.x, color.y, color.z, color.w} });
    }
    // Corrected push_back call
    vertices.push_back({ {coneTip.x, coneTip.y, coneTip.z}, {color.x, color.y, color.z, color.w} }); // Tip of cone
}


std::shared_ptr<VertexBuffer> CameraGizmo::CreateVertexBuffer(const GraphicsResourceDesc& resourceDesc)
{
    std::vector<Vertex> vertices;

    // X-Axis Arrow (Red)
    CreateArrowVertices(vertices, Vector3(1, 0, 0), Vector4(1, 0, 0, 1));
    // Y-Axis Arrow (Green)
    CreateArrowVertices(vertices, Vector3(0, 1, 0), Vector4(0, 1, 0, 1));
    // Z-Axis Arrow (Blue)
    CreateArrowVertices(vertices, Vector3(0, 0, 1), Vector4(0, 0, 1, 1));

    return std::make_shared<VertexBuffer>(
        vertices.data(),
        sizeof(Vertex),
        static_cast<ui32>(vertices.size()),
        resourceDesc
    );
}
// Helper function to create the indices for a single arrow
void CreateArrowIndices(std::vector<ui32>& indices, ui32 baseVertexOffset)
{
    const ui32 segments = SEGMENTS;
    ui32 cylinderVertexCount = (segments + 1) * 2;

    // cylinder Indices
    for (ui32 i = 0; i < segments; ++i)
    {
        ui32 currentBase = baseVertexOffset + i * 2;
        ui32 nextBase = baseVertexOffset + (i + 1) * 2;
        indices.push_back(currentBase);
        indices.push_back(nextBase + 1);
        indices.push_back(currentBase + 1);
        indices.push_back(currentBase);
        indices.push_back(nextBase);
        indices.push_back(nextBase + 1);
    }

    // cone Indices (Cone)
    ui32 coneBaseCenter = baseVertexOffset + cylinderVertexCount;
    ui32 coneTip = coneBaseCenter + segments + 2;
    for (ui32 i = 0; i < segments; ++i)
    {
        ui32 current = coneBaseCenter + 1 + i;
        ui32 next = coneBaseCenter + 1 + i + 1;
        // Base
        indices.push_back(coneBaseCenter);
        indices.push_back(next);
        indices.push_back(current);
        // Side
        indices.push_back(current);
        indices.push_back(next);
        indices.push_back(coneTip);
    }
}
std::shared_ptr<IndexBuffer> CameraGizmo::CreateIndexBuffer(const GraphicsResourceDesc& resourceDesc)
{
    std::vector<ui32> indices;

    const ui32 verticesPerArrow = (SEGMENTS + 1) * 2 + (SEGMENTS + 1) + 2;

    // X-Axis Arrow
    CreateArrowIndices(indices, 0);
    // Y-Axis Arrow
    CreateArrowIndices(indices, verticesPerArrow);
    // Z-Axis Arrow
    CreateArrowIndices(indices, verticesPerArrow * 2);

    return std::make_shared<IndexBuffer>(
        indices.data(),
        static_cast<ui32>(indices.size()),
        resourceDesc
    );
}