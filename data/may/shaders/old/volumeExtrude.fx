
float4x4 matrixWV : WORLD;
float4x4 matrixP : VIEWPROJECTION;

float4 lightPositionRangeCS;


float4 VS(
	float3 inPos		: POSITION,
    float3 inNormal		: NORMAL) : POSITION
{
	float3 pos = mul(float4(inPos, 1), matrixWV).xyz;
    float3 normal = normalize(mul(inNormal, (float3x3)matrixWV));

	float3 direction = pos - lightPositionRangeCS.xyz;
    if (dot(normal, direction) > 0)
    {
		float len = length(direction);
		float extrudeAmount = max(0, lightPositionRangeCS.w * 1.1 - len);			// 1.1 = extrude a bit further than light range to prevent artifacts with large faces
		pos += direction * (extrudeAmount/len);
	}
	return mul(float4(pos, 1), matrixP);
}


technique Basic
{
	pass P0
	{
		VertexShader = compile vs_2_0 VS();
		PixelShader  = null;
	}
}


