#version 400
layout(location=0) in vec4 gl_Vertex;
layout(location=1) in vec3 gl_Normal;

out vec3 v_p;
void main()
{
	gl_Position=gl_Vertex.xzyw;
	gl_Position.xyz*=0.05;
	v_p=gl_Normal.xyz*0.1+0.5;
}
