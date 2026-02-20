#type vertex
#version 460

layout (location = 0) in vec4 a_WSPosAndLSPos;
layout (location = 1) in vec4 a_ColorAndIntensity;
layout (location = 2) in float a_Radius;
layout (location = 3) in float a_Falloff;
layout (location = 4) in float a_Radians;

uniform mat4 u_ViewProjection;

out vec4 v_WSPosAndLSPos;
out vec4 v_ColorAndIntensity;
out float v_Radius;
out float v_Falloff;
out float v_Radians;

void main()
{
	gl_Position = u_ViewProjection * vec4(a_WSPosAndLSPos.xy, 0, 1.0f);

    v_WSPosAndLSPos = a_WSPosAndLSPos;
    v_ColorAndIntensity = a_ColorAndIntensity;
    v_Radius = a_Radius;
    v_Falloff = a_Falloff;
    v_Radians = a_Radians;
}

#type fragment
#version 460


in vec4 v_WSPosAndLSPos;
in vec4 v_ColorAndIntensity;
in float v_Radius;
in float v_Falloff;
in float v_Radians;

out vec4 color;

uniform vec2 u_ScreenSize;

uniform sampler2D u_Position;
uniform sampler2D u_Normal;
uniform sampler2D u_Albedo;


// Bresenham line algorithm for shadow casting
// Returns shadow attenuation factor (0.0 = fully shadowed, 1.0 = no shadow)
float SampleShadowAlongLine(vec2 startPos, vec2 endPos, float PixelDepth, sampler2D albedoSampler, vec2 startUV, vec2 screenSize)
{
    // Convert world positions to integer grid coordinates
    ivec2 x0 = ivec2(floor(startPos));
    ivec2 x1 = ivec2(floor(endPos));

    ivec2 delta = abs(x1 - x0);
    ivec2 step = ivec2(sign(x1 - x0));

    // Determine which axis is the major axis
    int err = delta.x - delta.y;

    ivec2 currentPos = x0;
    float shadowAttenuation = 1.0;

    // Maximum iterations to prevent infinite loops
    int maxSteps = max(delta.x, delta.y) + 1;

    for (int i = 0; i < maxSteps; i++)
    {
        vec2 vec = (currentPos - startPos) / screenSize;
        // Sample albedo at this position
        vec2 sampleUV = startUV + vec;
        vec4 albedoSample = texture(albedoSampler, sampleUV);

        // Check if this pixel is opaque (low alpha or bright enough to block light)
        // Adjust the threshold based on your needs
        float opacity = albedoSample.a;

        shadowAttenuation -= (opacity * (1.0f/PixelDepth));

        // Reduce shadow attenuation based on opacity encountered
        // This dims the light for each opaque pixel in the path
        //shadowAttenuation *= (1.0 - opacity * 0.1); // 10% reduction per opaque pixel

        // Stop if we've reached the end position
        if (currentPos == x1)
            break;

        // Bresenham stepping logic
        int e2 = 2 * err;

        if (e2 > -delta.y)
        {
            err -= delta.y;
            currentPos.x += step.x;
        }

        if (e2 < delta.x)
        {
            err += delta.x;
            currentPos.y += step.y;
        }
    }

    return shadowAttenuation;
}


void main()
{
    vec2 uv = vec2(gl_FragCoord.xy / u_ScreenSize);
	vec4 Src_Position = texture(u_Position, uv);
	vec4 Src_Normal = texture(u_Normal, uv);
	vec4 Src_Albedo = texture(u_Albedo, uv);
    float RadialFalloff = pow(clamp(1 - length(v_WSPosAndLSPos.zw), 0, 1), v_Falloff);

    vec2 VecToLight = (v_WSPosAndLSPos.zw * -v_Radius);
    vec2 LightSourceWorldPos = v_WSPosAndLSPos.xy + VecToLight;  
    //float Angle = atan(normalize(v_WSPosAndLSPos.zw).x, normalize(v_WSPosAndLSPos.zw).y);//gets angle from 0 being up in radians
    //Angle = Angle * sign(v_WSPosAndLSPos.x);//negative becomes positive again.
    //float AngularFalloff = 1 - smoothstep(0, v_Radians / 2, Angle);

    vec2 dirToLight = normalize(v_WSPosAndLSPos.xy - Src_Position.xy);
    float normalLight = clamp(dot(Src_Normal.xy, dirToLight), 0, 1);

    // Sample shadow along the line from light source to fragment
    float shadowAttenuation = SampleShadowAlongLine(Src_Position.xy,LightSourceWorldPos, 10, u_Albedo, uv, u_ScreenSize);

    // Apply shadow attenuation to the final color
    color = vec4(v_ColorAndIntensity.xyz, 1) * v_ColorAndIntensity.w * RadialFalloff * Src_Albedo * shadowAttenuation;

}
