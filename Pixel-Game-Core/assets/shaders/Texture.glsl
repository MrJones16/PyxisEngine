#type vertex
#version 460
			
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec4 a_Color;
layout (location = 2) in vec2 a_TexCoord;
layout (location = 3) in float a_TexIndex;
layout (location = 4) in float a_TilingFactor;
layout (location = 5) in uint a_NodeID;

uniform mat4 u_ViewProjection;

out vec4 v_Color;
out vec2 v_TexCoord;
out float v_TexIndex;
out float v_TilingFactor;
out flat uint v_NodeID;

void main()
{
	v_Color = a_Color;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0f);
	v_TexCoord = a_TexCoord;
	v_TexIndex = a_TexIndex;
	v_TilingFactor = a_TilingFactor;
	v_NodeID = a_NodeID;
}

#type fragment
#version 460
			
layout (location = 0) out vec4 color;
layout (location = 1) out uint id;

in vec4 v_Color;
in vec2 v_TexCoord;
in float v_TexIndex;
in float v_TilingFactor;
in flat uint v_NodeID;

uniform sampler2D u_Textures[32];

void main()
{
	vec4 result = texture(u_Textures[int(v_TexIndex)], v_TexCoord * v_TilingFactor) * v_Color;
	if(result.a == 0)
		discard;
	color = result;
	id = v_NodeID;
}