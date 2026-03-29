#pragma once

#include "ResolvePass.h"

namespace Templates
{
	struct ResolveUIPass : public ResolvePass
	{
		ResolveUIPass(CameraID cam, unsigned int rpI, RenderPassJsonID rpT, RenderPassInstanceID rp);
		virtual void CreatePrevPassDependentResources();
	};
};