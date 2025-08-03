#include <DX3D/Game/SelectionSystem.h>
#include <DX3D/Graphics/Primitives/AGameObject.h>
#include <DX3D/Game/SceneCamera.h>
#include <DirectXMath.h>
#include <limits>

using namespace dx3d;
using namespace DirectX;

SelectionSystem::SelectionSystem()
{
}

SelectionSystem::~SelectionSystem()
{
}

void SelectionSystem::setSelectedObject(std::shared_ptr<AGameObject> object)
{
    m_selectedObject = object;
}

std::shared_ptr<AGameObject> SelectionSystem::pickObject(
    const std::vector<std::shared_ptr<AGameObject>>& objects,
    const SceneCamera& camera,
    float mouseX, float mouseY,
    ui32 viewportWidth, ui32 viewportHeight)
{
    float ndcX = (2.0f * mouseX) / viewportWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * mouseY) / viewportHeight;

    XMVECTOR rayClip = XMVectorSet(ndcX, ndcY, -1.0f, 1.0f);

    float aspectRatio = static_cast<float>(viewportWidth) / viewportHeight;
    XMMATRIX projMatrix = Matrix4x4::CreatePerspectiveFovLH(1.0472f, aspectRatio, 0.1f, 100.0f).toXMMatrix();
    XMMATRIX invProj = XMMatrixInverse(nullptr, projMatrix);

    XMVECTOR rayView = XMVector4Transform(rayClip, invProj);
    rayView = XMVectorSet(XMVectorGetX(rayView), XMVectorGetY(rayView), -1.0f, 0.0f);

    XMMATRIX invView = XMMatrixInverse(nullptr, camera.getViewMatrix().toXMMatrix());
    XMVECTOR rayWorld = XMVector4Transform(rayView, invView);
    XMVECTOR rayDir = XMVector3Normalize(rayWorld);

    Vector3 rayOrigin = camera.getPosition();
    Vector3 rayDirection;
    XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&rayDirection), rayDir);

    std::shared_ptr<AGameObject> closestObject = nullptr;
    float closestT = std::numeric_limits<float>::max();

    for (const auto& object : objects)
    {
        Vector3 objectPos = object->getPosition();
        Vector3 aabbMin = objectPos - Vector3(0.5f, 0.5f, 0.5f);
        Vector3 aabbMax = objectPos + Vector3(0.5f, 0.5f, 0.5f);

        float t;
        if (rayIntersectsAABB(rayOrigin, rayDirection, aabbMin, aabbMax, t))
        {
            if (t < closestT)
            {
                closestT = t;
                closestObject = object;
            }
        }
    }

    return closestObject;
}

bool SelectionSystem::rayIntersectsAABB(const Vector3& rayOrigin, const Vector3& rayDir,
    const Vector3& aabbMin, const Vector3& aabbMax, float& t)
{
    float tmin = 0.0f;
    float tmax = std::numeric_limits<float>::max();

    for (int i = 0; i < 3; i++)
    {
        float origin = (i == 0) ? rayOrigin.x : (i == 1) ? rayOrigin.y : rayOrigin.z;
        float dir = (i == 0) ? rayDir.x : (i == 1) ? rayDir.y : rayDir.z;
        float min = (i == 0) ? aabbMin.x : (i == 1) ? aabbMin.y : aabbMin.z;
        float max = (i == 0) ? aabbMax.x : (i == 1) ? aabbMax.y : aabbMax.z;

        if (std::abs(dir) < 0.0001f)
        {
            if (origin < min || origin > max)
                return false;
        }
        else
        {
            float t1 = (min - origin) / dir;
            float t2 = (max - origin) / dir;

            if (t1 > t2) std::swap(t1, t2);

            tmin = std::max(tmin, t1);
            tmax = std::min(tmax, t2);

            if (tmin > tmax)
                return false;
        }
    }

    t = tmin;
    return true;
}