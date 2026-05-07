#pragma once

#include <SimpleMath.h>
#include <nlohmann/json.hpp>

using namespace DirectX;

namespace Scene::CameraProjections {

	struct Orthographic {
		static constexpr float defaultNearZ = 0.01f;
		static constexpr float defaultFarZ = 1000.0f;
		static constexpr float defaultViewLeft = 1000.0f;
		static constexpr float defaultViewRight = 0.0f;
		static constexpr float defaultViewTop = 1000.0f;
		static constexpr float defaultViewBottom = 0.0f;

		float nearZ = defaultNearZ;
		float farZ = defaultFarZ;
		float viewLeft = defaultViewLeft;
		float viewRight = defaultViewRight;
		float viewTop = defaultViewTop;
		float viewBottom = defaultViewBottom;
		XMMATRIX projectionMatrix;

		Orthographic() {}
		Orthographic(float nearZ, float farZ, float viewLeft, float viewRight, float viewTop, float viewBottom)
		{
			this->nearZ = nearZ;
			this->farZ = farZ;
			this->viewLeft = viewLeft;
			this->viewRight = viewRight;
			this->viewTop = viewTop;
			this->viewBottom = viewBottom;
			updateProjectionMatrix();
		}

		void Copy(Orthographic& other) {
			nearZ = other.nearZ;
			farZ = other.farZ;
			viewLeft = other.viewLeft;
			viewRight = other.viewRight;
			viewTop = other.viewTop;
			viewBottom = other.viewBottom;
			projectionMatrix = other.projectionMatrix;
		}

		inline void updateProjectionMatrix() {
			projectionMatrix = XMMatrixOrthographicOffCenterLH(viewLeft, viewRight, viewBottom, viewTop, nearZ, farZ);
		}

		inline void updateProjectionMatrix(float viewLeft, float viewRight, float viewBottom, float viewTop) {
			this->viewLeft = viewLeft;
			this->viewRight = viewRight;
			this->viewBottom = viewBottom;
			this->viewTop = viewTop;
			updateProjectionMatrix();
		};

		inline void updateNearZ(float nearZ) {
			this->nearZ = nearZ;
			updateProjectionMatrix();
		}

		inline void updateFarZ(float farZ) {
			this->farZ = farZ;
			updateProjectionMatrix();
		}

		inline void expandView(float diff) {
			viewLeft = std::clamp(viewLeft + diff, 4.0f, 200.0f);
			viewRight = std::clamp(viewRight + diff, 4.0f, 200.0f);
			viewBottom = std::clamp(viewBottom + diff, 4.0f, 200.0f);
			viewTop = std::clamp(viewTop + diff, 4.0f, 200.0f);
			updateProjectionMatrix();
		}

	};

	inline Orthographic ToOrthographic(nlohmann::json& j)
	{
		Orthographic p;
		p.nearZ = static_cast<float>(j.at("nearZ"));
		p.farZ = static_cast<float>(j.at("farZ"));
		p.viewLeft = static_cast<float>(j.at("viewLeft"));
		p.viewRight = static_cast<float>(j.at("viewRight"));
		p.viewBottom = static_cast<float>(j.at("viewBottom"));
		p.viewTop = static_cast<float>(j.at("viewTop"));
		return p;
	}

	inline nlohmann::json FromOrthographic(Orthographic p)
	{
		return {
			{ "nearZ", p.nearZ },
			{ "farZ", p.farZ },
			{ "viewLeft", p.viewLeft },
			{ "viewRight", p.viewRight },
			{ "viewBottom", p.viewBottom },
			{ "viewTop", p.viewTop },
		};
	}
};
using namespace Scene::CameraProjections;

inline void to_json(nlohmann::json& j, const Orthographic& p) {
	j = FromOrthographic(p);
}

inline void from_json(const nlohmann::json& j, Orthographic& p) {
	p = ToOrthographic(const_cast<nlohmann::json&>(j));
}