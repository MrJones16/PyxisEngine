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


void main()
{
    vec2 uv = vec2(gl_FragCoord.xy / u_ScreenSize);
	vec4 Src_Position = texture(u_Position, uv);
	vec4 Src_Normal = texture(u_Normal, uv);
	vec4 Src_Albedo = texture(u_Albedo, uv);

    float RadialFalloff = pow(clamp(1 - length(v_WSPosAndLSPos.zw), 0, 1), v_Falloff);

    //float Angle = atan(normalize(v_WSPosAndLSPos.zw).x, normalize(v_WSPosAndLSPos.zw).y);//gets angle from 0 being up in radians
    //Angle = Angle * sign(v_WSPosAndLSPos.x);//negative becomes positive again.
    //float AngularFalloff = 1 - smoothstep(0, v_Radians / 2, Angle);

    vec2 dirToLight = normalize(v_WSPosAndLSPos.xy - Src_Position.xy);
    float normalLight = clamp(dot(Src_Normal.xy, dirToLight), 0, 1);

    color = vec4(v_ColorAndIntensity.xyz, 1) * v_ColorAndIntensity.w * RadialFalloff * Src_Albedo;
    
}
