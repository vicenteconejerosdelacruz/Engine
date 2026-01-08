#pragma once

#include "OverridePass.h"

struct ResolvePass : public OverridePass
{
	enum ResolveMode
	{
		ResolveMode_FullScreenQuad,
		ResolveMode_CopyFromRenderToTexture
	};

	ResolveMode mode;

	ResolvePass(SceneUnitId id, JUUID cam, unsigned int rpI, JUUID rp);
	virtual void Pass(SceneUnitId unit);
	void Render(SceneUnitId unit);
};

