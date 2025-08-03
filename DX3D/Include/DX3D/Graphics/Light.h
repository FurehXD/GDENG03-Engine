#pragma once
#include <DX3D/Math/Math.h>

namespace dx3d
{
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

    __declspec(align(16)) struct Light
    {
        Vector3 position;
        int type = LIGHT_TYPE_DIRECTIONAL;

        Vector3 direction;
        float intensity = 1.0f;

        Vector3 color;
        float radius = 10.0f;

        float spot_angle_inner = 0.785f;
        float spot_angle_outer = 0.959f;
        float spot_falloff = 1.0f; 
        float padding; 
    };

#define MAX_LIGHTS_SUPPORTED 16
    __declspec(align(16)) struct LightConstantBuffer
    {
        Vector4 camera_position;
        Vector4 ambient_color;
        UINT num_lights;
        Vector3 padding;
        Light lights[MAX_LIGHTS_SUPPORTED];
    };
}