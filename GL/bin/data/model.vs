#version 400
#extension GL_ARB_explicit_uniform_location : enable

#define float3 vec3
#define float2 vec2
#define float4 vec4

#define float4x4 mat4

layout(location=0) in float4 i_pos;
layout(location=1) in float3 i_nor;
layout(location=2) in float3 i_tan;
layout(location=3) in float3 i_bin;
layout(location=4) in float2 i_tex;

layout(location=0) uniform float4x4 u_wm;
layout(location=4) uniform float4x4 u_vpm;


out float3 v_pos;
out float3 v_nor;
out float3 v_tan;
out float3 v_bin;
out float2 v_tex;

void main()
{
	float4 wpos=i_pos*u_wm;
	v_pos = wpos.xyz;
	gl_Position=wpos*u_vpm;
	v_nor =(float4(i_nor,0.0)*u_wm).xyz;
	v_tan =(float4(i_tan,0.0)*u_wm).xyz;
	v_bin =(float4(i_bin,0.0)*u_wm).xyz;
	v_tex =i_tex;
}
