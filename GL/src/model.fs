#version 400
#extension GL_ARB_explicit_uniform_location : enable

layout(location=6) uniform sampler2D sTex;
layout(location=1) uniform sampler2D sNor;

in vec3 v_p;
in vec2 v_t;
void main()
{
	vec3 n=normalize(v_p);
	float l=dot(normalize(vec3(1,1,1)),-n)*0.5+0.5;
	vec4 color=texture(sTex, v_t.xy);
	vec4 norm=texture(sNor, v_t.xy);
	color.rgb*=norm.rgb;
//	color.xy=v_t.xy;
    gl_FragColor=color;//vec4(l,l,l,1.0);
}
