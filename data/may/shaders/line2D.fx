
float4 PS( float4 color : COLOR0) : COLOR0
{
	return color;
}

technique Basic
{
    pass P0
    {
        VertexShader = null;
        PixelShader  = compile ps_2_0 PS();
    }
}


