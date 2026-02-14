#include "pch.h"
#include "Pentahedron.h"

static std::vector<uint32_t> indices = {
	 0, 1, 2,
	 3, 4, 5,
	 6, 7, 8,
	 9,10,11,
	12,13,14,
	15,16,17
};

std::vector<uint32_t> Pentahedron::GetIndices()
{
	return indices;
}

static std::vector<VertexPosTexCoord> vertices = {
	{ XMFLOAT3(-1.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
	{ XMFLOAT3(0.0f, 2.0f,  0.0f), XMFLOAT2(0.5f, 0.0f) },
	{ XMFLOAT3(1.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

	{ XMFLOAT3(1.0f, 0.0f,  1.0f), XMFLOAT2(1.0f, 1.0f) },
	{ XMFLOAT3(0.0f, 2.0f,  0.0f), XMFLOAT2(0.5f, 0.0f) },
	{ XMFLOAT3(-1.0f, 0.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) },

	{ XMFLOAT3(1.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
	{ XMFLOAT3(0.0f, 2.0f,  0.0f), XMFLOAT2(0.5f, 0.0f) },
	{ XMFLOAT3(1.0f, 0.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) },

	{ XMFLOAT3(-1.0f, 0.0f,  1.0f), XMFLOAT2(1.0f, 1.0f) },
	{ XMFLOAT3(0.0f, 2.0f,  0.0f), XMFLOAT2(0.5f, 0.0f) },
	{ XMFLOAT3(-1.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

	{ XMFLOAT3(1.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(1.0f, 0.0f,  1.0f), XMFLOAT2(1.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, 0.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) },

	{ XMFLOAT3(1.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(-1.0f, 0.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
};

std::vector<Pentahedron::VertexType> Pentahedron::GetVertices()
{
	std::vector<Pentahedron::VertexType> pentVertices;

	for (unsigned int i = 0; i < vertices.size(); i += 3)
	{
		VertexPosTexCoord& v0 = vertices.at(i);
		VertexPosTexCoord& v1 = vertices.at(i + 1);
		VertexPosTexCoord& v2 = vertices.at(i + 2);

		XMVECTOR p0 = XMLoadFloat3(&v0.Position);
		XMVECTOR p1 = XMLoadFloat3(&v1.Position);
		XMVECTOR p2 = XMLoadFloat3(&v2.Position);
		XMVECTOR uv0 = XMLoadFloat2(&v0.TexCoord);
		XMVECTOR uv1 = XMLoadFloat2(&v1.TexCoord);
		XMVECTOR uv2 = XMLoadFloat2(&v2.TexCoord);

		XMVECTOR dp1 = XMVectorSubtract(p1, p0);
		XMVECTOR dp2 = XMVectorSubtract(p2, p0);
		XMVECTOR duv1 = XMVectorSubtract(uv1, uv0);
		XMVECTOR duv2 = XMVectorSubtract(uv2, uv0);

		float r = 1.0f / (duv1.m128_f32[0] * duv2.m128_f32[1] - duv1.m128_f32[1] * duv2.m128_f32[0]);
		XMVECTOR t0 = XMVectorScale(XMVectorScale(dp1, dp2.m128_f32[1]), r);
		XMVECTOR t1 = XMVectorScale(XMVectorScale(dp2, dp1.m128_f32[1]), -r);
		XMVECTOR t = XMVector3Normalize(XMVectorAdd(t0, t1));

		//hack as bottom faces are giving us bad tangents
		if (t.m128_f32[0] == 0.0f && t.m128_f32[1] == 0.0f && t.m128_f32[2] == 0.0f)
		{
			t = duv2;
		}

		XMVECTOR normal = XMVector3Normalize(XMVector3Cross(XMVectorSubtract(p1, p2), XMVectorSubtract(p1, p0)));

		XMFLOAT3 mNormal, mt;
		XMStoreFloat3(&mNormal, normal);
		XMStoreFloat3(&mt, t);

		VertexPosNormalTangentTexCoord pv0 = { .Position = v0.Position, .Normal = mNormal, .Tangent = mt, .TexCoord = v0.TexCoord };
		VertexPosNormalTangentTexCoord pv1 = { .Position = v1.Position, .Normal = mNormal, .Tangent = mt, .TexCoord = v1.TexCoord };
		VertexPosNormalTangentTexCoord pv2 = { .Position = v2.Position, .Normal = mNormal, .Tangent = mt, .TexCoord = v2.TexCoord };

		pentVertices.push_back(pv0);
		pentVertices.push_back(pv1);
		pentVertices.push_back(pv2);
	}

	return pentVertices;
}
