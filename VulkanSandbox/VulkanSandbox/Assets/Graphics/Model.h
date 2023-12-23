#pragma once
#include <vk_mem_alloc.h>

#include "../Asset.h"
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

class Model : public Asset
{
	public:
	Model(const std::filesystem::path &path);
	~Model();

	void Load() override;
	void Unload() override;
	bool IsLoaded() const override;


	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;

		static vk::VertexInputBindingDescription getBindingDescription()
		{
			vk::VertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = vk::VertexInputRate::eVertex;

			return bindingDescription;
		}

		static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions()
		{
			std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions{};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
			attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

			return attributeDescriptions;
		}
	};
	friend class Graphics;
private:
	void DrawCmd();
	//TODO: Use one vk buffer per model for vertices and indices
	std::vector<Vertex> _vertices;
	std::vector<uint32_t> _indices;

	vk::Buffer _vertexBuffer;
	VmaAllocation _vertexBufferMemory;
	vk::Buffer _indexBuffer;
	VmaAllocation _indexBufferMemory;
	
	std::filesystem::path _modelPath;

	bool _loaded = false;

	void CreateVertexBuffer();
	void CreateIndexBuffer();
};