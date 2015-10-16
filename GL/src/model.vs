#version 400
layout(location=0) in vec4 i_pos;
layout(location=1) in vec3 i_norm;
layout(location=2) in vec3 i_tan;
layout(location=3) in vec3 i_bin;
layout(location=4) in vec2 i_tex;


out vec3 v_p;
out vec2 v_t;
void main()
{
	gl_Position=i_pos.xzyw;
	gl_Position.xyz*=0.05;
	gl_Position.z*=0.1;
	v_p=i_norm.xzy;
	v_t=i_tex;
}
