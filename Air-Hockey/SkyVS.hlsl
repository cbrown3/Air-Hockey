
// Constant Buffer for external (C++) data
cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
};

// Struct representing a single vertex worth of data
struct VertexShaderInput
{
	float3 position		: POSITION;
	float2 uv			: TEXCOORD;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
};

// Out of the vertex shader (and eventually input to the PS)
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 uvw			: TEXCOORD0;
};

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// --------------------------------------------------------
VertexToPixel main(VertexShaderInput input)
{
	// Set up output
	VertexToPixel output;

	// The view matrix without translations
	matrix viewNoMovement = view;
	viewNoMovement._41 = 0;
	viewNoMovement._42 = 0;
	viewNoMovement._43 = 0;

	// Calculate output position
	matrix vp = mul(viewNoMovement, projection);
	output.position = mul(float4(input.position, 1.0f), vp);

	// Ensure the vertex's depth is going to be 1.0 exactly
	output.position.z = output.position.w;

	// Send down the direction to this vertex from the origin
	// (which happens to just be it's raw position)
	output.uvw = input.position;

	return output;
}