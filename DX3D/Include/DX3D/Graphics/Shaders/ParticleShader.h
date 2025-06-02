#pragma once
#include <DX3D/Graphics/Shaders/Shaders.h>

namespace dx3d
{
    class ParticleShader
    {
    public:
        static const char* GetVertexShaderCode()
        {
            return R"(
                struct VS_INPUT {
                    float3 position : POSITION;
                    float4 color : COLOR;
                };
                
                struct VS_OUTPUT {
                    float4 position : SV_POSITION;
                    float4 color : COLOR;
                };
                
                VS_OUTPUT main(VS_INPUT input) {
                    VS_OUTPUT output;
                    output.position = float4(input.position, 1.0f);
                    output.color = input.color;
                    return output;
                }
            )";
        }

        static const char* GetPixelShaderCode()
        {
            return R"(
                struct PS_INPUT {
                    float4 position : SV_POSITION;
                    float4 color : COLOR;
                };
                
                float4 main(PS_INPUT input) : SV_TARGET {
                    // Simply output the interpolated color with alpha
                    return input.color;
                }
            )";
        }
    };
}