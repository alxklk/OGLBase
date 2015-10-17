#version 400

#define float3 vec3
#define float2 vec2
#define float4 vec4
#define float4x4 mat4

uniform sampler2D sTex;
uniform sampler2D sNor;

uniform float3 lightPos0;
uniform float3 lightPos1;
uniform float3 lightPos2;

uniform float3 lightCol0;
uniform float3 lightCol1;
uniform float3 lightCol2;


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
