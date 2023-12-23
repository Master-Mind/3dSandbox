#include "Model.h"
#include "../../Utils/CLogger.h"

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "../../Graphics/Graphics.h"

Model::Model(const std::filesystem::path& path) : _modelPath(path)
{
}

Model::~Model()
{
    if (_loaded)
	{
		Unload();
	}
}

void Model::DrawCmd()
{
}

void Model::Load()
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile("Data/Models/viking_room.obj", aiProcess_Triangulate | aiProcess_FlipUVs);

    Assert(scene, "Could not load model");

    for (unsigned i = 0; i < scene->mNumMeshes; ++i)
    {
        aiMesh* curMesh = scene->mMeshes[i];

        for (unsigned j = 0; j < curMesh->mNumVertices; ++j)
        {
            Vertex curVer{};

            curVer.pos = { curMesh->mVertices[j].x, curMesh->mVertices[j].y, curMesh->mVertices[j].z };
            curVer.color = { 1, 1, 1 };
            curVer.texCoord = { curMesh->mTextureCoords[0][j].x, curMesh->mTextureCoords[0][j].y };

            _vertices.push_back(curVer);
        }

        for (unsigned int i = 0; i < curMesh->mNumFaces; i++)
        {
            aiFace face = curMesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                _indices.push_back(face.mIndices[j]);
        }
    }

    CreateVertexBuffer();
    CreateIndexBuffer();
}

void Model::Unload()
{
    vmaDestroyBuffer(Graphics::_allocator, _indexBuffer, _indexBufferMemory);
    vmaDestroyBuffer(Graphics::_allocator, _vertexBuffer, _vertexBufferMemory);
}

bool Model::IsLoaded() const
{
    return _loaded;
}

void Model::CreateVertexBuffer()
{
    vk::DeviceSize bufferSize = sizeof(_vertices[0]) * _vertices.size();

    vk::Buffer stagingBuffer;
    VmaAllocation stagingBufferMemory;
    Graphics::CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        stagingBuffer, stagingBufferMemory, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    void* data;
    VkResult result = vmaMapMemory(Graphics::_allocator, stagingBufferMemory, &data);

    if (result != VK_SUCCESS)
    {
        Error("Failed to map vertex memory!");
        return;
    }

    memcpy(data, _vertices.data(), bufferSize);
    vmaUnmapMemory(Graphics::_allocator, stagingBufferMemory);

    Graphics::CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal, _vertexBuffer,
        _vertexBufferMemory);

    Graphics::CopyBuffer(stagingBuffer, _vertexBuffer, bufferSize);

    vmaDestroyBuffer(Graphics::_allocator, stagingBuffer, stagingBufferMemory);
}

void Model::CreateIndexBuffer()
{
    vk::DeviceSize bufferSize = sizeof(_indices[0]) * _indices.size();

    vk::Buffer stagingBuffer;
    VmaAllocation stagingBufferMemory;
    Graphics::CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        stagingBuffer, stagingBufferMemory, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    void* data;
    VkResult result = vmaMapMemory(Graphics::_allocator, stagingBufferMemory, &data);

    if (result != VK_SUCCESS)
    {
        Error("Failed to map index memory!");
        return;
    }

    memcpy(data, _indices.data(), bufferSize);
    vmaUnmapMemory(Graphics::_allocator, stagingBufferMemory);

    Graphics::CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal, _indexBuffer,
        _indexBufferMemory);

    Graphics::CopyBuffer(stagingBuffer, _indexBuffer, bufferSize);

    vmaDestroyBuffer(Graphics::_allocator, stagingBuffer, stagingBufferMemory);
}
