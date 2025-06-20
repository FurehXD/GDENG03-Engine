#pragma once
#include <DX3D/Graphics/Shaders/Shaders.h>

namespace dx3d
{
    class Rainbow3DShader
    {
    public:
        static const char* GetVertexShaderCode()
        {
            return R"(
                cbuffer TransformBuffer : register(b0)
                {
                    matrix world;
                    matrix view;
                    matrix projection;
                };

                struct VS_INPUT {
                    float3 position : POSITION;
                    float4 color : COLOR;
                };
                
                struct VS_OUTPUT {
                    float4 position : SV_POSITION;
                    float3 objectPos : TEXCOORD0;  // Pass object-space position
                    float3 worldPos : TEXCOORD1;   // Keep world pos for optional effects
                };
                
                VS_OUTPUT main(VS_INPUT input) {
                    VS_OUTPUT output;
                    
                    // Store the original object-space position for rainbow calculation
                    output.objectPos = input.position;
                    
                    // Transform the position from object space to world space
                    float4 worldPosition = mul(float4(input.position, 1.0f), world);
                    output.worldPos = worldPosition.xyz;
                    
                    // Transform from world space to view space
                    float4 viewPosition = mul(worldPosition, view);
                    
                    // Transform from view space to projection space
                    output.position = mul(viewPosition, projection);
                    
                    return output;
                }
            )";
        }

        static const char* GetPixelShaderCode()
        {
            return R"(
                struct PS_INPUT {
                    float4 position : SV_POSITION;
                    float3 objectPos : TEXCOORD0;
                    float3 worldPos : TEXCOORD1;
                };
                
                // Enhanced HSV to RGB conversion for vibrant rainbow colors
                float3 hsv2rgb(float3 c) {
                    float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
                    float3 p = abs(frac(c.xxx + K.xyz) * 6.0 - K.www);
                    return c.z * lerp(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
                }
                
                float4 main(PS_INPUT input) : SV_TARGET {
                    // Use object-space coordinates for consistent rainbow pattern
                    float3 objPos = input.objectPos;
                    
                    // Create rainbow based on object coordinates (not world position)
                    // This ensures consistent colors regardless of object position/scale
                    
                    // Method 1: Horizontal rainbow based on X coordinate
                    // Map X coordinate from [-0.5, 0.5] to [0, 1] for hue
                    float hue = (objPos.x + 0.5) * 2.0; // Assuming object coords are roughly [-0.5, 0.5]
                    
                    // Alternative Method 2: Diagonal rainbow (uncomment to use)
                    // float hue = (objPos.x + objPos.z + 1.0) * 0.5;
                    
                    // Alternative Method 3: Radial rainbow from center (uncomment to use)
                    // float distance = length(objPos.xz);
                    // float hue = distance * 2.0;
                    
                    // Alternative Method 4: Spiral rainbow (uncomment to use)
                    // float angle = atan2(objPos.z, objPos.x);
                    // float hue = (angle + 3.14159) / (2.0 * 3.14159); // Map angle to [0,1]
                    // hue += objPos.y * 0.5; // Add vertical component
                    
                    // Keep hue in [0,1] range
                    hue = frac(hue);
                    
                    // High saturation and brightness for vivid colors
                    float saturation = 1.0;
                    float brightness = 0.95;
                    
                    // Optional: Add subtle variation based on surface normal approximation
                    // This can add some depth while maintaining consistent base colors
                    float normalVariation = sin(objPos.y * 10.0) * 0.05;
                    brightness += normalVariation;
                    brightness = clamp(brightness, 0.5, 1.0);
                    
                    float3 rainbowColor = hsv2rgb(float3(hue, saturation, brightness));
                    
                    // Boost the colors for maximum vibrancy
                    rainbowColor = saturate(rainbowColor * 1.1);
                    
                    return float4(rainbowColor, 1.0);
                }
            )";
        }
    };
}