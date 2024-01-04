#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

#include "ShDevice.h"

class ShDescriptorSetLayout {
public:
	class Builder {
	public:
		Builder(ShDevice& shDevice) : shDevice{ shDevice } {}

		Builder& addBinding(
			uint32_t binding,
			VkDescriptorType descriptorType,
			VkShaderStageFlags stageFlags,
			uint32_t count = 1);
		std::unique_ptr<ShDescriptorSetLayout> build() const;

	private:
		ShDevice& shDevice;
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
	};

	ShDescriptorSetLayout(
		ShDevice& ShDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
	~ShDescriptorSetLayout();
	ShDescriptorSetLayout(const ShDescriptorSetLayout&) = delete;
	ShDescriptorSetLayout& operator=(const ShDescriptorSetLayout&) = delete;

	VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

private:
	ShDevice& shDevice;
	VkDescriptorSetLayout descriptorSetLayout;
	std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

	friend class ShDescriptorWriter;
};


class ShDescriptorPool {
public:
	class Builder {
	public:
		Builder(ShDevice& shDevice) : shDevice{ shDevice } {}

		Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
		Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
		Builder& setMaxSets(uint32_t count);
		std::unique_ptr<ShDescriptorPool> build() const;

	private:
		ShDevice& shDevice;
		std::vector<VkDescriptorPoolSize> poolSizes{};
		uint32_t maxSets = 1000;
		VkDescriptorPoolCreateFlags poolFlags = 0;
	};

	ShDescriptorPool(
		ShDevice& ShDevice,
		uint32_t maxSets,
		VkDescriptorPoolCreateFlags poolFlags,
		const std::vector<VkDescriptorPoolSize>& poolSizes);
	~ShDescriptorPool();
	ShDescriptorPool(const ShDescriptorPool&) = delete;
	ShDescriptorPool& operator=(const ShDescriptorPool&) = delete;

	bool allocateDescriptor(
		const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

	void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

	void resetPool();

private:
	ShDevice& shDevice;
	VkDescriptorPool descriptorPool;

	friend class ShDescriptorWriter;
};

class ShDescriptorWriter {
public:
	ShDescriptorWriter(ShDescriptorSetLayout& setLayout, ShDescriptorPool& pool);

	ShDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
	ShDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

	bool build(VkDescriptorSet& set);
	void overwrite(VkDescriptorSet& set);

private:
	ShDescriptorSetLayout& setLayout;
	ShDescriptorPool& pool;
	std::vector<VkWriteDescriptorSet> writes;
};


