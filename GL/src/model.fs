#version 400
#extension GL_ARB_explicit_uniform_location : enable

#define float3 vec3
#define float2 vec2
#define float4 vec4
#define float4x4 mat4

layout(location=5) uniform sampler2D sTex;
layout(location=6) uniform sampler2D sNor;


in float3 v_pos;
in float3 v_norm;
in float3 v_tan;
in float3 v_bin;
in float2 v_tex;

out float4 o_col;

void main()
{
	float3 norm=texture(sNor, v_tex.xy).xyz;
    norm=norm*2.0-1.0;
	float3 n=normalize(v_norm)*norm.z+normalize(v_tan)*norm.x+normalize(v_bin)*norm.y;
    n=normalize(n);

	float l0=clamp(dot(normalize(float3(1,-0.5,2)),n),0.0,1.0);
	float l1=clamp(dot(normalize(float3(-1,-1,1)),n),0.0,1.0);
	float l2=clamp(dot(normalize(float3(-1,-1,-1)),n),0.0,1.0);
    float3 l=float3(1.0,0.5,0.2)*l0+float3(0.5,1.0,0.5)*l1+float3(0.4,0.5,0.9)*l2;
	float4 color=texture(sTex, v_tex.xy);
//	color.rgb*=norm.rgb;
//	color.xy=v_t.xy;
    discard(color.a<0.15);
//    o_col=vec4(color.rgb,1.0); return;
    o_col=vec4(l*0.8,1.0); return;
    o_col=vec4(color.rgb*l,1.0); 
}
