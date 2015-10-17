#version 400
#extension GL_ARB_explicit_uniform_location : enable

#define float3 vec3
#define float2 vec2
#define float4 vec4
#define float4x4 mat4

layout(location=5) uniform sampler2D sTex;
layout(location=6) uniform sampler2D sNor;

layout(location=7) uniform float3 lightPos0;
layout(location=8) uniform float3 lightPos1;
layout(location=9) uniform float3 lightPos2;

layout(location=10) uniform float3 lightCol0;
layout(location=11) uniform float3 lightCol1;
layout(location=12) uniform float3 lightCol2;


in float3 v_pos;
in float3 v_nor;
in float3 v_tan;
in float3 v_bin;
in float2 v_tex;

out float4 o_col;

void main()
{
	float3 norm=texture(sNor, v_tex.xy).xyz;
	norm=norm*2.0-1.0;
	float3 n=normalize(v_nor)*norm.z+normalize(v_tan)*norm.x+normalize(v_bin)*norm.y;
	n=normalize(n);

	float3 tolight0=v_pos-lightPos0;
	float3 tolight1=v_pos-lightPos1;
	float3 tolight2=v_pos-lightPos2;

	float l0=clamp(dot(normalize(tolight0),n),0.0,1.0);
	float l1=clamp(dot(normalize(tolight1),n),0.0,1.0);
	float l2=clamp(dot(normalize(tolight2),n),0.0,1.0);

	float3 l=
		lightCol0*l0+
		lightCol1*l1+
		lightCol2*l2;

	float4 color=texture(sTex, v_tex.xy);
	o_col=vec4(color.rgb*l,color.a); 
}
