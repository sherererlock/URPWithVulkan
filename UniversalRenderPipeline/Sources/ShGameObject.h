#pragma once

#include <memory>
#include <unordered_map>

#include "ShModel.h"
#include "VulkanglTFModel.h"

#include <glm/glm/gtc/matrix_transform.hpp>

using namespace vkglTF;

struct TransformComponent {
	glm::vec3 translation{};
	glm::vec3 scale{ 1.f, 1.f, 1.f };
	glm::vec3 rotation{};

	// Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
	// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
	// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
	glm::mat4 mat4();

	glm::mat3 normalMatrix();
};

struct PointLightComponent {
	float lightIntensity = 1.0f;
};

class ShGameObject {
public:
	using id_t = unsigned int;
	using Map = std::unordered_map<id_t, ShGameObject>;

	static ShGameObject createGameObject() {
		static id_t currentId = 0;
		return ShGameObject{ currentId++ };
	}

	static ShGameObject makePointLight(
		float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

	ShGameObject(const ShGameObject&) = delete;
	ShGameObject& operator=(const ShGameObject&) = delete;
	ShGameObject(ShGameObject&&) = default;
	ShGameObject& operator=(ShGameObject&&) = default;

	id_t getId() { return id; }

	glm::vec3 color{};
	TransformComponent transform{};

	// Optional pointer components
	std::shared_ptr<ShModel> model{};
	std::shared_ptr<Model> gltfmodel{};
	std::unique_ptr<PointLightComponent> pointLight = nullptr;

private:
	ShGameObject(id_t objId) : id{ objId } {}

	id_t id;
};

