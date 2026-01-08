#pragma once
#include "OverridePass.h"
//#include <HDR/LuminanceHistogram.h>
//#include <HDR/LuminanceHistogramAverage.h>

//namespace ComputeShader
//{
//	struct LuminanceHistogram;
//	struct LuminanceHistogramAverage;
//};

struct ToneMappingPass : public OverridePass
{
	//std::unique_ptr<ComputeShader::LuminanceHistogram> hdrHistogram;
	//std::unique_ptr<ComputeShader::LuminanceHistogramAverage> luminanceHistogramAverage;

	ToneMappingPass(SceneUnitId id, JUUID cam, unsigned int rpI, JUUID rp);
	virtual ~ToneMappingPass();
	virtual void Pass(SceneUnitId unit);
	void Render();
};

