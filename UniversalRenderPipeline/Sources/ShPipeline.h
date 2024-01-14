#pragma once

#include <string>
#include <vector>
#include <functional>

#include "ShDevice.h"

struct PipelineConfigInfo 
{
	PipelineConfigInfo() = default;
	PipelineConfigInfo(const PipelineConfigInfo&) = delete;
	PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

	std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
	VkPipelineViewportStateCreateInfo viewportInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	VkPipelineRasterizationStateCreateInfo rasterizationInfo;
	VkPipelineMultisampleStateCreateInfo multisampleInfo;
	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
	VkPipelineColorBlendStateCreateInfo colorBlendInfo;
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
	std::vector<VkDynamicState> dynamicStateEnables;
	VkPipelineDynamicStateCreateInfo dynamicStateInfo;
	VkPipelineLayout pipelineLayout = nullptr;
	VkRenderPass renderPass = nullptr;
	uint32_t subpass = 0;
};

class ShPipeline {
public:
	ShPipeline(
		ShDevice& device,
		const std::string& vertFilepath,
		const std::string& fragFilepath,
		const PipelineConfigInfo& configInfo);
	~ShPipeline();

	ShPipeline(const ShPipeline&) = delete;
	ShPipeline& operator=(const ShPipeline&) = delete;

	void bind(VkCommandBuffer commandBuffer);

	typedef std::function<std::vector<VkVertexInputBindingDescription>()> getInputBinding;
	typedef std::function<std::vector<VkVertexInputAttributeDescription>()> getInputAttribute;
	static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo, getInputBinding f1, getInputAttribute f2, uint32_t colorBlendAttachmentCount = 1);
	static void enableAlphaBlending(PipelineConfigInfo& configInfo);

private:
	static std::vector<char> readFile(const std::string& filepath);

	void createGraphicsPipeline(
		const std::string& vertFilepath,
		const std::string& fragFilepath,
		const PipelineConfigInfo& configInfo);

	void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

	ShDevice& shDevice;
	VkPipeline graphicsPipeline;
	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;
};

