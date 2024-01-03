/////////////////////////////////
// Vertex
/////////////////////////////////

#type vertex
#version 460
			
layout (location = 0) in vec3 a_Position;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

//out vec2 v_TexCoord;

void main()
{
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0f);
	//v_TexCoord = a_TexCoord;
}


/////////////////////////////////
// Fragment
/////////////////////////////////

#type fragment
#version 330 core

in vec2 gl_FragCoord;

out vec4 fragColor;

uniform vec3 u_CameraPosition;
uniform vec3 u_CameraRotation;
uniform mat4 u_RotationMatrix;
uniform vec2 u_Resolution;
uniform float u_FOV;

const int maxIterations = 100;
const float MAX_DISTANCE = 1000;



//Cube SDF
float sdfCube(vec3 p, vec3 size) {
    vec3 d = abs(p) - size;
    float insideDistance = min(max(d.x, max(d.y, d.z)), 0.0);
    float outsideDistance = length(max(d, 0.0));

    return insideDistance + outsideDistance;
}

// Function to calculate the signed distance to the scene
float scene(vec3 p) {
    // Example: Sphere
    
    float sphereDist = length(p) - 0.6;
    return max(sphereDist, sdfCube(p, vec3(0.5,0.5,0.5)));
}

vec3 GetNormal(vec3 p)
{
    vec2 d = vec2(0.01, 0.0);
    float gx = scene(p + d.xyy) - scene(p-d.xyy);
    float gy = scene(p + d.yxy) - scene(p-d.yxy);
    float gz = scene(p + d.yyx) - scene(p-d.yyx);
    vec3 normal = vec3(gx, gy, gz);
    return normalize(normal);
}

// Ray marching function
float rayMarch(vec3 origin, vec3 direction) {
    float t = 0.0;
    for (int i = 0; i < maxIterations; i++) {
        vec3 p = origin + t * direction;
        float distance = scene(p);
        
        // If we're close enough to the surface, return the accumulated distance
        if (distance < 0.001) {
            return t;
        }
        
        // Move along the ray by the signed distance
        t += distance;
        
        // Break if we're outside a bounding box
        if (t > MAX_DISTANCE) {
            break;
        }
    }
    
    return t;
}

void main() {
    vec3 lightColor = vec3(1,1,1);
    vec3 lightPos = vec3(4,1,-1);

    // Normalize fragment coordinates to the range [-1, 1]
    vec2 uv = (2.0 * gl_FragCoord - u_Resolution) / min(u_Resolution.x, u_Resolution.y);

    // Define the camera parameters
    vec3 cameraDir = normalize(vec3(uv, u_FOV));

    cameraDir = vec3(u_RotationMatrix * vec4(cameraDir, 0));
    
    // Perform ray marching
    float t = rayMarch(u_CameraPosition, cameraDir);
    
    // Color the pixel based on the distance traveled along the ray
    if (t < MAX_DISTANCE)
    {
        vec3 hitPos = u_CameraPosition + t * cameraDir;
        vec3 lightDir = lightPos - hitPos;
        float lightDist = length(lightDir);
        if (rayMarch(hitPos + 0.1 * normalize(lightDir), normalize(lightDir)) >= lightDist)
        {
            //light is hitting
            vec3 n = GetNormal(hitPos);
            float diffuse = max(0, dot(n, lightDir));
            fragColor = vec4((lightColor * diffuse * 0.8), 1);
            fragColor += vec4(lightColor * 0.1, 1);
        }else
        {
            //in shadow
            fragColor = vec4((lightColor * 0.1), 1);
        }
        //fragColor = vec4(n.x, n.y, n.z, 1);//normals coloring
    }
    else
    {
        fragColor = vec4(0,0,0,1);
    }
}