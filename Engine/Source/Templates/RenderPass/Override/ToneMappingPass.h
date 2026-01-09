#pragma once
#include "OverridePass.h"
#include <HDR/LuminanceHistogram.h>
#include <HDR/LuminanceHistogramAverage.h>

namespace ComputeShader
{
	struct LuminanceHistogram;
	struct LuminanceHistogramAverage;
};
using namespace ComputeShader;

struct ToneMappingPass : public OverridePass
{

	ToneMappingPass(SceneUnitId id, JUUID cam, unsigned int rpI, JUUID rp);
	virtual ~ToneMappingPass();
	virtual void Pass(SceneUnitId id);
	void Render(SceneUnitId id);

	std::unique_ptr<LuminanceHistogram> hdrHistogram;
	std::unique_ptr<LuminanceHistogramAverage> luminanceHistogramAverage;
};

