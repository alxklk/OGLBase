#version 400

#define float3 vec3
#define float2 vec2
#define float4 vec4

#define float4x4 mat4

in float4 i_pos;
in float3 i_nor;
in float3 i_tan;
in float3 i_bin;
in float2 i_tex;

uniform float4x4 u_wm;
uniform float4x4 u_vpm;

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
