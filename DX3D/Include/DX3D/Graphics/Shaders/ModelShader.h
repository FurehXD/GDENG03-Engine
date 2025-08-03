#pragma once
#include <DX3D/Graphics/Shaders/Shaders.h>
#include <DX3D/Math/Math.h>


namespace dx3d
{
    class ModelShader
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
                    float3 normal : NORMAL;
                    float2 texCoord : TEXCOORD;
                };

                struct VS_OUTPUT {
                    float4 position : SV_POSITION;
                    float4 color : COLOR;
                    float3 normal : NORMAL;
                    float2 texCoord : TEXCOORD0;
                    float3 worldPos : TEXCOORD1;
                };

                VS_OUTPUT main(VS_INPUT input) {
                    VS_OUTPUT output;

                    // Transform position to world space
                    float4 worldPosition = mul(float4(input.position, 1.0f), world);
                    output.worldPos = worldPosition.xyz;

                    // Transform to view space
                    float4 viewPosition = mul(worldPosition, view);

                    // Transform to projection space
                    output.position = mul(viewPosition, projection);

                    // Transform normal to world space
                    output.normal = normalize(mul(input.normal, (float3x3)world));

                    // Pass through color and texture coordinates
                    output.color = input.color;
                    output.texCoord = input.texCoord;

                    return output;
                }
            )";
        }

        static const char* GetPixelShaderCode()
        {
            return R"(
                #define LIGHT_TYPE_DIRECTIONAL 0
                #define LIGHT_TYPE_POINT 1
                #define LIGHT_TYPE_SPOT 2

                struct Light
                {
                    float3 position;
                    int    type;
                    float3 direction;
                    float  intensity;
                    float3 color;
                    float  radius;
                    float  spot_angle_inner;
                    float  spot_angle_outer;
                    float  spot_falloff;
                    float  padding;
                };

                cbuffer SceneBuffer : register(b2)
                {
                    float4 camera_position;
                    float4 ambient_color;
                    uint   num_lights;
                    float3 padding;
                    Light  lights[16];
                };
        
                cbuffer MaterialBuffer : register(b1)
                {
                    float4 diffuseColor;
                    float4 ambientColor;
                    float4 specularColor;
                    float4 emissiveColor;
                    float  specularPower;
                    float  opacity;
                    float  hasTexture;
                    float  padding_mat;
                };

                Texture2D diffuseTexture : register(t0);
                SamplerState textureSampler : register(s0);

                struct PS_INPUT {
                    float4 position     : SV_POSITION;
                    float4 color        : COLOR;
                    float3 normal       : NORMAL;
                    float2 texCoord     : TEXCOORD0;
                    float3 worldPos     : TEXCOORD1;
                };

                // --- THIS IS THE FULLY RESTORED AND CORRECTED FUNCTION ---
                float3 calculateLight(Light light, float3 pixel_world_pos, float3 normal, float3 view_dir)
                {
                    float3 light_dir;
                    float attenuation = 1.0f;
            
                    if (light.type == LIGHT_TYPE_DIRECTIONAL)
                    {
                        light_dir = normalize(-light.direction);
                        // No attenuation for directional lights
                    }
                    else // Point and Spot lights
                    {
                        float3 distance_vec = light.position - pixel_world_pos;
                        float dist = length(distance_vec);
                        light_dir = normalize(distance_vec);
                        attenuation = saturate(1.0f - (dist / light.radius));
                        attenuation *= attenuation; // Quadratic falloff for a smoother look
                    }

                    if (light.type == LIGHT_TYPE_SPOT)
                    {
                        float spot_factor = dot(normalize(-light.direction), -light_dir);
                        float spot_effect = smoothstep(light.spot_angle_outer, light.spot_angle_inner, spot_factor);
                        attenuation *= spot_effect;
                    }

                    // Diffuse Lighting (multiplied by the material's diffuse color)
                    float diffuse_factor = max(dot(normal, light_dir), 0.0f);
                    float3 diffuse = diffuse_factor * light.color * light.intensity * diffuseColor.rgb;

                    // Specular Lighting (multiplied by the material's specular color)
                    float3 halfway_vec = normalize(light_dir + view_dir);
                    float specular_factor = pow(max(dot(normal, halfway_vec), 0.0f), specularPower);
                    float3 specular = specular_factor * light.color * light.intensity * specularColor.rgb;
            
                    // Combine and apply attenuation
                    return (diffuse + specular) * attenuation;
                }

                // --- THIS IS THE FULLY RESTORED AND CORRECTED MAIN FUNCTION ---
                float4 main(PS_INPUT input) : SV_TARGET 
                {
                    float3 normal = normalize(input.normal);
                    float3 view_dir = normalize(camera_position.xyz - input.worldPos);
            
                    // Start with ambient light, respecting the material's ambient properties
                    float4 finalColor = ambientColor * ambient_color;

                    // Add the contribution from every active light
                    for (uint i = 0; i < num_lights; i++)
                    {
                        finalColor.rgb += calculateLight(lights[i], input.worldPos, normal, view_dir);
                    }

                    // Add any emissive color from the material itself
                    finalColor.rgb += emissiveColor.rgb;

                    // Apply texture if one exists
                    if (hasTexture > 0.5f) {
                        finalColor.rgb = finalColor.rgb * diffuseTexture.Sample(textureSampler, input.texCoord).rgb;
                    }

                    finalColor.a = diffuseColor.a * opacity;
                    return finalColor;
                }
            )";
        }
    };

    // Material constant buffer structure for models
    struct ModelMaterialConstants
    {
        Vector4 diffuseColor;
        Vector4 ambientColor;
        Vector4 specularColor;
        Vector4 emissiveColor;
        float specularPower;
        float opacity;
        float hasTexture;
        float padding;
    };
}