#pragma once

inline void PrintRect(std::string name, RECT& r)
{
	std::string title = "rect-" + name + "\n";
	std::string left = "left:" + std::to_string(r.left) + "\n";
	std::string right = "right:" + std::to_string(r.right) + "\n";
	std::string top = "top:" + std::to_string(r.top) + "\n";
	std::string bottom = "bottom:" + std::to_string(r.bottom) + "\n";
	OutputDebugStringA(title.c_str());
	OutputDebugStringA(left.c_str());
	OutputDebugStringA(right.c_str());
	OutputDebugStringA(top.c_str());
	OutputDebugStringA(bottom.c_str());
}

inline void PrintXMVector(XMVECTOR v, std::string name = "")
{
	OutputDebugStringA(std::string(name + std::string((name != "") ? ":" : "") + std::to_string(v.m128_f32[0]) + "," + std::to_string(v.m128_f32[1]) + "," + std::to_string(v.m128_f32[2]) + "," + std::to_string(v.m128_f32[3]) + "\n").c_str());
}

inline void PrintXMFloat3(XMFLOAT3 v, std::string name = "")
{
	OutputDebugStringA(std::string(name + std::string((name != "") ? ":" : "") + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + "\n").c_str());
}

inline std::string OutputV3(XMVECTOR V3)
{
	return std::string(std::to_string(V3.m128_f32[0]) + "," + std::to_string(V3.m128_f32[1]) + "," + std::to_string(V3.m128_f32[2]));
}

inline void MatrixDump(std::string matrixName, DirectX::XMMATRIX m)
{
	std::string row1 = "[" +
		std::to_string(m.r[0].m128_f32[0]) + "," +
		std::to_string(m.r[0].m128_f32[1]) + "," +
		std::to_string(m.r[0].m128_f32[2]) + "," +
		std::to_string(m.r[0].m128_f32[3]) + "]";
	std::string row2 = "[" +
		std::to_string(m.r[1].m128_f32[0]) + "," +
		std::to_string(m.r[1].m128_f32[1]) + "," +
		std::to_string(m.r[1].m128_f32[2]) + "," +
		std::to_string(m.r[1].m128_f32[3]) + "]";
	std::string row3 = "[" +
		std::to_string(m.r[2].m128_f32[0]) + "," +
		std::to_string(m.r[2].m128_f32[1]) + "," +
		std::to_string(m.r[2].m128_f32[2]) + "," +
		std::to_string(m.r[2].m128_f32[3]) + "]";
	std::string row4 = "[" +
		std::to_string(m.r[3].m128_f32[0]) + "," +
		std::to_string(m.r[3].m128_f32[1]) + "," +
		std::to_string(m.r[3].m128_f32[2]) + "," +
		std::to_string(m.r[3].m128_f32[3]) + "]";
	std::string matrixDump = row1 + "\n" + row2 + "\n" + row3 + "\n" + row4 + "\n";

	OutputDebugStringA((matrixName + "\n").c_str());
	OutputDebugStringA(matrixDump.c_str());
}

inline void MatrixDump(std::string matrixName, DirectX::XMFLOAT4X4 m)
{
	std::string row1 = "[" +
		std::to_string(m._11) + "," +
		std::to_string(m._12) + "," +
		std::to_string(m._13) + "," +
		std::to_string(m._14) + "]";
	std::string row2 = "[" +
		std::to_string(m._21) + "," +
		std::to_string(m._22) + "," +
		std::to_string(m._23) + "," +
		std::to_string(m._24) + "]";
	std::string row3 = "[" +
		std::to_string(m._31) + "," +
		std::to_string(m._32) + "," +
		std::to_string(m._33) + "," +
		std::to_string(m._34) + "]";
	std::string row4 = "[" +
		std::to_string(m._41) + "," +
		std::to_string(m._42) + "," +
		std::to_string(m._43) + "," +
		std::to_string(m._44) + "]";
	std::string matrixDump = row1 + "\n" + row2 + "\n" + row3 + "\n" + row4 + "\n";

	OutputDebugStringA((matrixName + "\n").c_str());
	OutputDebugStringA(matrixDump.c_str());
}

