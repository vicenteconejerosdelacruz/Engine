#pragma once

#include <SimpleMath.h>

namespace Scene::CameraProjections {

	struct Perspective {
		static constexpr float defaultFovAngleY = 70.0f;
		static constexpr float defaultNearZ = 0.01f;
		static constexpr float defaultFarZ = 1000.0f;
		static constexpr float defaultWidth = 1000.0f;
		static constexpr float defaultHeight = 1000.0f;

		float nearZ = defaultNearZ;
		float farZ = defaultFarZ;
		float fovAngleY = defaultFovAngleY;
		float width = defaultWidth;
		float height = defaultHeight;
		XMMATRIX projectionMatrix;

		Perspective() {}
		Perspective(float nearZ, float farZ, float fovAngleY, float width, float height)
		{
			this->nearZ = nearZ;
			this->farZ = farZ;
			this->fovAngleY = fovAngleY;
			this->width = width;
			this->height = height;
			updateProjectionMatrix();
		}

		void Copy(Perspective& other) {
			nearZ = other.nearZ;
			farZ = other.farZ;
			fovAngleY = other.fovAngleY;
			width = other.width;
			height = other.height;
			projectionMatrix = other.projectionMatrix;
		}

		inline void updateProjectionMatrix() {
			float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
			projectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), aspectRatio, nearZ, farZ);
		};

		inline void updateProjectionMatrix(float width, float height) {
			this->width = width;
			this->height = height;
			updateProjectionMatrix();
		};

		inline void updateFovAngle(float fovAngle) {
			fovAngleY = fovAngle;
			updateProjectionMatrix(width, height);
		}

	};

	inline Perspective ToPerspective(nlohmann::json& j)
	{
		Perspective p;
		p.nearZ = static_cast<float>(j.at("nearZ"));
		p.farZ = static_cast<float>(j.at("farZ"));
		p.fovAngleY = static_cast<float>(j.at("fovAngleY"));
		if (j.contains("width")) p.width = static_cast<float>(j.at("width"));
		if (j.contains("height")) p.height = static_cast<float>(j.at("height"));
		return p;
	}

	inline nlohmann::json FromPerspective(Perspective p)
	{
		return {
			{ "nearZ", p.nearZ },
			{ "farZ", p.farZ },
			{ "fovAngleY", p.fovAngleY },
			{ "width", p.width },
			{ "height", p.height }
		};
	}
};