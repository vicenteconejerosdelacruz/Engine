#pragma once
#include <set>
#include <map>
#include <vector>
#include <memory>
#include <d3d12shader.h>
#include <wrl.h>
#include <wrl/client.h>
#include <dxcapi.h>
#include <string>
//#include <JTypes.h>
#include <Templates.h>
#include <JTemplate.h>
//#include <TemplateDecl.h>
#include "ShaderInstance.h"

namespace Templates
{
	inline static const std::string ShadowMapLightsShaderResourceViewName = TextureShaderUsageToString.at(TextureShaderUsage_ShadowMaps);

#if defined(_EDITOR)

#include <Attributes/JOrder.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Editor/JDrawersDecl.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Editor/JPreviewDecl.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Creator/JRequired.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Creator/JJsonDecl.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Creator/JDrawersDecl.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Creator/JValidatorDecl.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#endif

#if defined(_DEVELOPMENT)
	void ShaderJsonStep();
	void MonitorShaderChanges(std::string folder);
#endif

	namespace Shader
	{
		inline static const std::string templateName = "shaders.json";
		inline static const TemplateType templateType = T_Shaders;
	};

	struct ShaderJson : public JTemplate
	{
		TEMPLATE_DECL(Shader);

#include <Attributes/JFlags.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#include <Attributes/JDecl.h>
#include <ShaderAtt.h>
#include <JEnd.h>

#if defined(_EDITOR)
		virtual void WriteJson(nlohmann::json& j);
#endif
	};

	TEMPDECL_FULL(Shader);
	TEMPDECL_REFTRACKER(Shader);
}
