#include "pch.h"
#include "ResolveUIPass.h"
#include <Scene.h>
#include <SceneObject.h>
#include <DeviceUtils/RenderToTexture/RenderToTexture.h>
#include <DeviceUtils/RenderPass/SwapChainPass.h>

namespace Templates
{
	ResolveUIPass::ResolveUIPass(CameraID cam, unsigned int rpI, RenderPassJsonID rpT, RenderPassInstanceID rp) : ResolvePass(cam, rpI, rpT, rp)
	{
		CreatePrevPassDependentResources();
	}

	void ResolveUIPass::CreatePrevPassDependentResources()
	{
		mode = ResolveMode_CopyFromRenderToTexture;

		CreateFsQuadResources(camera.unit(), "FullScreenUIQuad", renderPassTemplate(), [this](std::string name, ShaderConstantsBufferVariable& var)
			{
				auto& fsCB = fsQuadConstantsBuffer;

				for (unsigned int n = 0; n < JRenderer::numFrames; n++)
				{
					if (name == "alpha")
					{
						float data = 1.0f;
						fsCB->push(data, n, var.offset);
					}
				}
			}
		);
	}
}
