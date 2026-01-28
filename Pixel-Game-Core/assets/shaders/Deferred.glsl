#type vertex
#version 460
			
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec4 a_Albedo;
layout (location = 3) in vec2 a_TexCoord;
layout (location = 4) in float a_TexIndex;
layout (location = 5) in float a_TilingFactor;
layout (location = 6) in uint a_NodeID;

uniform mat4 u_ViewProjection;

out vec4 v_Position;
out vec4 v_Normal;
out vec4 v_Albedo;
out vec2 v_TexCoord;
out float v_TexIndex;
out float v_TilingFactor;
out flat uint v_NodeID;

void main()
{
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0f);
	v_Position = u_ViewProjection * vec4(a_Position, 1.0f);

	v_Normal = vec4(a_Normal, 1.0f);
	v_Albedo = a_Albedo;
	v_TexCoord = a_TexCoord;
	v_TexIndex = a_TexIndex;
	v_TilingFactor = a_TilingFactor;
	v_NodeID = a_NodeID;
}

#type fragment
#version 460
			
layout (location = 0) out vec4 o_Position;
layout (location = 1) out vec4 o_Normal;
layout (location = 2) out vec4 o_Albedo;
layout (location = 3) out uint o_ID;

in vec4 v_Position;
in vec4 v_Normal;
in vec4 v_Albedo;
in vec2 v_TexCoord;
in float v_TexIndex;
in float v_TilingFactor;
in flat uint v_NodeID;

uniform sampler2D u_Textures[32];

void main()
{
	vec4 result = texture(u_Textures[int(v_TexIndex)], v_TexCoord * v_TilingFactor) * v_Albedo;
	if(result.a == 0)
		discard;

    o_Position = v_Position;
    o_Normal = v_Normal;
	o_Albedo = result;
	o_ID = v_NodeID;
}
