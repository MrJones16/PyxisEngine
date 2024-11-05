//https://www.shadertoy.com/view/3sKXDK

#type vertex
#version 460
			
layout (location = 0) in vec3 a_Position;

out vec3 v_Position;

void main()
{
	gl_Position = vec4(a_Position, 1.0f);
	v_Position = a_Position;
}

#type fragment
#version 460

layout (location = 0) out vec4 fragColor;

// Amanatides 3D DDA marching implementation - Paper: http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.42.3443&rep=rep1&type=pdf
// By SebH 
// https://twitter.com/SebHillaire
// Use mouse left to rotate camera (X axis)
// Morphed from https://www.shadertoy.com/view/MdlyDs
//
// WARNING: 
// Does not compile on all platforms/driver due to the packedBunny array which is
// a ugly hacky way to get volume asset texture in shader toy. 
// A right way to do that would be to have loadable textures from weblinks.
// Set "VOLUME_FILTERING_NEAREST 1" can fix that issue.
//

#define float2 vec2
#define float3 vec3
#define float4 vec4
#define uint2 uvec2
#define uint3 uvec3
#define uint4 uvec4

//////////////////////////////////////////////////
// Bunny volume data
//////////////////////////////////////////////////

// Packed 32^3 bunny data as 32x32 uint where each bit represents density per voxel
#define BUNNY_VOLUME_SIZE 32
const uint packedBunny[1024] = uint[1024](0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 917504u, 917504u, 917504u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 1966080u, 12531712u, 16742400u, 16742400u, 16723968u, 16711680u, 8323072u, 4128768u, 2031616u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 6144u, 2063360u, 16776704u, 33553920u, 33553920u, 33553920u, 33553920u, 33520640u, 16711680u, 8323072u, 8323072u, 2031616u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 268435456u, 402653184u, 134217728u, 201326592u, 67108864u, 0u, 0u, 7168u, 2031104u, 16776960u, 33554176u, 33554176u, 33554304u, 33554176u, 33554176u, 33554176u, 33553920u, 16744448u, 8323072u, 4128768u, 1572864u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 805306368u, 939524096u, 402653184u, 478150656u, 260046848u, 260046848u, 260046848u, 125832192u, 130055680u, 67108608u, 33554304u, 33554304u, 33554304u, 33554304u, 33554304u, 33554304u, 33554304u, 33554176u, 16776704u, 8355840u, 4128768u, 917504u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 805306368u, 1056964608u, 1056964608u, 528482304u, 528482304u, 260046848u, 260046848u, 260046848u, 130039296u, 130154240u, 67108739u, 67108807u, 33554375u, 33554375u, 33554370u, 33554368u, 33554368u, 33554304u, 33554304u, 16776960u, 8330240u, 4128768u, 393216u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 939524096u, 1040187392u, 1040187392u, 520093696u, 251658240u, 251658240u, 260046848u, 125829120u, 125829120u, 130088704u, 63045504u, 33554375u, 33554375u, 33554375u, 33554407u, 33554407u, 33554370u, 33554370u, 33554374u, 33554310u, 16776966u, 4144642u, 917504u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 15360u, 130816u, 262017u, 4194247u, 33554383u, 67108847u, 33554415u, 33554407u, 33554407u, 33554375u, 33554375u, 33554318u, 2031502u, 32262u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 31744u, 130816u, 262019u, 2097151u, 134217727u, 134217727u, 67108863u, 33554415u, 33554407u, 33554415u, 33554383u, 2097102u, 982926u, 32262u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 31744u, 130816u, 524263u, 117964799u, 127926271u, 134217727u, 67108863u, 16777215u, 4194303u, 4194303u, 2097151u, 1048574u, 65422u, 16134u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 3u, 31751u, 130951u, 524287u, 252182527u, 261095423u, 261095423u, 59768830u, 2097150u, 1048574u, 1048575u, 262143u, 131070u, 65534u, 16134u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 7u, 31751u, 130959u, 503840767u, 520617982u, 529530879u, 261095423u, 1048575u, 1048574u, 1048574u, 524286u, 524287u, 131070u, 65534u, 16134u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 3u, 1799u, 32527u, 134348750u, 1040449534u, 1057488894u, 520617982u, 51380223u, 1048575u, 1048575u, 524287u, 524287u, 524287u, 131070u, 65534u, 15886u, 6u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 1536u, 3968u, 8175u, 65535u, 1006764030u, 1040449534u, 1057488894u, 50855934u, 524286u, 524286u, 524287u, 524287u, 524286u, 262142u, 131070u, 65534u, 32270u, 14u, 6u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 3968u, 8160u, 8191u, 805371903u, 2080505854u, 2114191358u, 101187582u, 34078718u, 524286u, 524286u, 524286u, 524286u, 524286u, 524286u, 262142u, 131070u, 32766u, 8078u, 3590u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 8128u, 8176u, 16383u, 2013331455u, 2080505854u, 235143166u, 101187582u, 524286u, 1048574u, 1048574u, 1048574u, 1048574u, 524286u, 524286u, 262142u, 131070u, 32766u, 16382u, 8070u, 1024u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 8160u, 8184u, 1879064574u, 2013331455u, 470024190u, 67371006u, 524286u, 1048574u, 1048574u, 1048574u, 1048574u, 1048574u, 1048574u, 524286u, 524286u, 262142u, 65534u, 16382u, 8160u, 1024u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 8128u, 8184u, 805322750u, 402718719u, 134479870u, 524286u, 524286u, 1048574u, 1048574u, 1048574u, 1048574u, 1048574u, 1048574u, 1048574u, 524286u, 262142u, 65534u, 16382u, 16368u, 1792u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 3968u, 8184u, 16382u, 131071u, 262142u, 524286u, 1048574u, 1048574u, 1048574u, 1048574u, 1048574u, 1048574u, 1048574u, 1048574u, 524286u, 262142u, 65534u, 16382u, 16368u, 1792u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 1792u, 8184u, 16380u, 65535u, 262143u, 524286u, 524286u, 1048574u, 1048574u, 1048575u, 1048574u, 1048574u, 1048574u, 1048574u, 524286u, 262142u, 65534u, 16376u, 16368u, 1792u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 8176u, 16376u, 32767u, 262143u, 524286u, 1048574u, 1048574u, 1048575u, 1048575u, 1048575u, 1048575u, 1048574u, 1048574u, 524286u, 262142u, 32766u, 16376u, 8176u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 4032u, 8184u, 32766u, 262142u, 524286u, 524286u, 1048575u, 1048574u, 1048574u, 1048574u, 1048574u, 1048574u, 1048574u, 524286u, 262142u, 32766u, 16376u, 8176u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 384u, 8184u, 32766u, 131070u, 262142u, 524286u, 1048575u, 1048574u, 1048574u, 1048574u, 1048574u, 1048574u, 524286u, 524286u, 131070u, 32766u, 16368u, 1920u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 4080u, 32764u, 65534u, 262142u, 524286u, 524286u, 524286u, 1048574u, 1048574u, 524286u, 524286u, 524286u, 262142u, 131070u, 32764u, 8160u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 256u, 16376u, 32760u, 131068u, 262140u, 262142u, 524286u, 524286u, 524286u, 524286u, 524286u, 262142u, 131070u, 65532u, 16368u, 3840u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 3968u, 32752u, 65528u, 131068u, 262142u, 262142u, 262142u, 262142u, 262142u, 262142u, 262140u, 131064u, 32752u, 7936u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 8064u, 32736u, 65528u, 131070u, 131070u, 131070u, 131070u, 131070u, 131070u, 65532u, 32752u, 8160u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 3456u, 16376u, 32764u, 65534u, 65534u, 65534u, 32766u, 32764u, 16380u, 4048u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 48u, 2680u, 8188u, 8188u, 8188u, 8188u, 4092u, 120u, 16u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 120u, 248u, 508u, 508u, 508u, 248u, 240u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 96u, 240u, 504u, 504u, 504u, 240u, 96u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 224u, 224u, 224u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u);

float sampleBunny(float3 uvs)
{
    float3 voxelUvs = max(float3(0.0), min(uvs * float3(BUNNY_VOLUME_SIZE), float3(BUNNY_VOLUME_SIZE) - 1.0));
    uint3 intCoord = uint3(voxelUvs);
    uint arrayCoord = intCoord.x + intCoord.z * uint(BUNNY_VOLUME_SIZE);

    // Very simple clamp to edge. It would be better to do it for each texture sample
    // before the filtering but that would be more expenssive...
    // Also adding small offset to catch cube intersection floating point error
    if (uvs.x < -0.001 || uvs.y < -0.001 || uvs.z < -0.001 ||
        uvs.x>1.001 || uvs.y>1.001 || uvs.z>1.001)
        return 0.0;

    // sample the uint representing a packed volume data of 32 voxel (1 or 0)
    uint bunnyDepthData = packedBunny[arrayCoord];
    float voxel = (bunnyDepthData & (1u << intCoord.y)) > 0u ? 1.0 : 0.0;

    return voxel;
}

//////////////////////////////////////////////////
// Cube intersection
//////////////////////////////////////////////////

bool slabs(float3 p0, float3 p1, float3 rayOrigin, float3 invRaydir, out float outTMin, out float outTMax)
{
    float3 t0 = (p0 - rayOrigin) * invRaydir;
    float3 t1 = (p1 - rayOrigin) * invRaydir;
    float3 tmin = min(t0, t1), tmax = max(t0, t1);
    float maxtmin = max(max(tmin.x, tmin.y), tmin.z);
    float mintmax = min(min(tmax.x, tmax.y), tmax.z);
    outTMin = maxtmin;
    outTMax = mintmax;
    return maxtmin <= mintmax;
}

//////////////////////////////////////////////////
// MAIN
//////////////////////////////////////////////////

uniform vec2 u_Resolution;
uniform sampler3D u_ColorTexture;

void main()
{
    float2 fragCoord = gl_FragCoord.xy;
    float2 uv = fragCoord.xy / u_Resolution.xy;
    fragColor = float4(uv, 0.5, 1.0);

    // View diretion in camera space
    float3 viewDir = normalize(float3((fragCoord.xy - u_Resolution.xy * 0.5) / u_Resolution.y, 1.0));

    // Compute camera properties
    float  camDist = 1.1;
    float3 camUp = float3(0, 1.0, 0);
    float3 camPos = float3(camDist, 1.1, camDist);
    float3 camTarget = float3(0, 0.0, 0);

    // And from them evaluted ray direction in world space
    float3 forward = normalize(camTarget - camPos);
    float3 left = normalize(cross(forward, camUp));
    float3 up = cross(left, forward);
    float3 worldDir = viewDir.x * left + viewDir.y * up + viewDir.z * forward;

    float3 color = float3(0.0, 0.0, 0.0);

    //////////////////////////////////////////////////////////////////////////////////////////
    //// Compute intersection with cube containing the bunny
    float near = 0.0;
    float far = 0.0;
    float3 D = normalize(worldDir);
    if (slabs(float3(-0.5), float3(0.5), camPos, 1.0 / D, near, far))
    {
        float3 StartPos = camPos + D * near;

        StartPos = (StartPos + 0.5) * 32.0;
        // Round to axis aligned volume
        if (StartPos.x < 0.0) StartPos.x = 0.0f;
        if (StartPos.y < 0.0) StartPos.y = 0.0f;
        if (StartPos.z < 0.0) StartPos.z = 0.0f;
        if (StartPos.x > 32.0) StartPos.x = 32.0;
        if (StartPos.y > 32.0) StartPos.y = 32.0;
        if (StartPos.z > 32.0) StartPos.z = 32.0;

        float3 P = StartPos;

        // Amanatides 3D-DDA data preparation
        float3 stepSign = sign(D);
        float3 tDelta = abs(1.0 / D);
        float3 tMax = float3(0.0, 0.0, 0.0);
        float3 refPoint = floor(P);
        tMax.x = stepSign.x > 0.0 ? refPoint.x + 1.0 - P.x : P.x - refPoint.x; // floor is more consistent than ceil
        tMax.y = stepSign.y > 0.0 ? refPoint.y + 1.0 - P.y : P.y - refPoint.y;
        tMax.z = stepSign.z > 0.0 ? refPoint.z + 1.0 - P.z : P.z - refPoint.z;
        tMax.x *= tDelta.x;
        tMax.y *= tDelta.y;
        tMax.z *= tDelta.z;


        const float LowB = -0.01;
        const float HighB = 32.01;
        while (P.x >= LowB && P.y >= LowB && P.z >= LowB && P.x <= HighB && P.y <= HighB && P.z <= HighB)
        {
#if 0
            // Slow reference
            P += D * 0.005;
#else
            // Amanatides 3D-DDA 
            if (tMax.x < tMax.y)
            {
                if (tMax.x < tMax.z)
                {
                    P.x += stepSign.x;
                    tMax.x += tDelta.x;
                }
                else
                {
                    P.z += stepSign.z;
                    tMax.z += tDelta.z;
                }
            }
            else
            {
                if (tMax.y < tMax.z)
                {
                    P.y += stepSign.y;
                    tMax.y += tDelta.y;
                }
                else
                {
                    P.z += stepSign.z;
                    tMax.z += tDelta.z;
                }
            }
#endif

            float3 Voxel = floor(P);
            if (sampleBunny(float3(Voxel) / 32.0f) > 0.0)
            {
                //color = texture(u_ColorTexture, Voxel * 1.0 / 32.0 + 0.5 / 32.0).rgb;
                color = vec3(1,1,1);
                break;
            }
        }

        // Debug
        //color = fract(StartPos);
        //color = float3(far-near,0,0);
        //color = abs(sin(worldDir*10.0));

    }

    fragColor = float4(pow(color, float3(1.0 / 2.2)), 1.0); // simple linear to gamma


}


