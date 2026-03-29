#pragma once

#include "OverridePass.h"

namespace Templates
{
	struct ResolvePass : public OverridePass
	{
		enum ResolveMode
		{
			ResolveMode_FullScreenQuad,
			ResolveMode_CopyFromRenderToTexture
		};

		ResolveMode mode;
		RenderToTextureID rt_texture;
		bool clearRTV = true;

		ResolvePass(CameraID cam, unsigned int rpI, RenderPassJsonID rpT, RenderPassInstanceID rp);
		virtual void CreatePrevPassDependentResources();
		virtual void Pass(SceneUnitId unit);
		void Render(SceneUnitId unit);
	};
};