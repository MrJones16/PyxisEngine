#type vertex
#version 460
			
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec2 v_TexCoord;

void main()
{
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0f);
	v_TexCoord = a_TexCoord;
}

#type fragment
#version 460
			
layout (location = 0) out vec4 color;

uniform sampler2D u_TextureDiffuse;

in vec2 v_TexCoord;

void main()
{
	color = texture(u_TextureDiffuse, v_TexCoord);
}