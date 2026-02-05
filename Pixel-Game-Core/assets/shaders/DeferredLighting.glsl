#type vertex
#version 460
			
layout (location = 0) in vec4 a_WSPosAndLSPos;
layout (location = 1) in vec4 a_ColorAndIntensity;
layout (location = 2) in float a_Radius;
layout (location = 3) in float a_Intensity;
layout (location = 4) in float a_Falloff;
layout (location = 5) in float a_MinAngle;
layout (location = 6) in float a_MaxAngle;

uniform mat4 u_ViewProjection;

out vec4 v_WSPosAndLSPos;
out vec4 v_ColorAndIntensity;
out float v_Radius;
out float v_Intensity;
out float v_Falloff;
out float v_MinAngle;
out float v_MaxAngle;

void main()
{
	gl_Position = u_ViewProjection * vec4(a_WSPosAndLSPos.xy, 0, 1.0f);

    v_WSPosAndLSPos = a_WSPosAndLSPos;
    v_ColorAndIntensity = a_ColorAndIntensity;
    v_Radius = a_Radius;
    v_Falloff = a_Falloff;
    v_MinAngle = a_MinAngle;
    v_MaxAngle = a_MaxAngle;
}

#type fragment
#version 460

			
in vec4 v_WSPosAndLSPos;
in vec4 v_ColorAndIntensity;
in float v_Radius;
in float v_Falloff;
in float v_MinAngle;
in float v_MaxAngle;

out vec4 color;

uniform vec2 u_ScreenSize;

uniform sampler2D u_Position;
uniform sampler2D u_Normal;
uniform sampler2D u_Albedo;


void main()
{
    vec2 uv = vec2(gl_FragCoord.xy / u_ScreenSize);
	vec4 Src_Position = texture(u_Position, uv);
	vec4 Src_Normal = texture(u_Normal, uv);
	vec4 Src_Albedo = texture(u_Albedo, uv);
    float radialFalloff = pow(length(v_WSPosAndLSPos.zw), v_Falloff);
    color = vec4(radialFalloff, 0, 0, 1);
}
