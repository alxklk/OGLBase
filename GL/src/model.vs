#version 400
#extension GL_ARB_explicit_uniform_location : enable

#define float3 vec3
#define float2 vec2
#define float4 vec4

#define float4x4 mat4

layout(location=0) in float4 i_pos;
layout(location=1) in float3 i_norm;
layout(location=2) in float3 i_tan;
layout(location=3) in float3 i_bin;
layout(location=4) in float2 i_tex;

layout(location=0) uniform float4x4 u_wm;
layout(location=1) uniform float4x4 u_vpm;


out float3 v_pos;
out float3 v_norm;
out float3 v_tan;
out float3 v_bin;
out float2 v_tex;

void main()
{
	gl_Position=i_pos.xzyw*u_wm;
	gl_Position.xyz*=0.05;
	gl_Position.z*=0.1;
	v_pos =i_pos.xyz;
	v_norm=(float4(i_norm,0.0)*u_wm).xzy;
	v_tan =(float4(i_tan,0.0)*u_wm).xzy;
	v_bin =(float4(i_bin,0.0)*u_wm).xzy;
	v_tex =i_tex;
}
