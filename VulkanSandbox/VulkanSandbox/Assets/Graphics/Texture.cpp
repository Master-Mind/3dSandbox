#include "Texture.h"

#include <stb_image.h>
#include "../../Utils/CLogger.h""

Texture::Texture(const std::filesystem::path& path) : _path(path)
{
}

Texture::~Texture()
{
    if (IsLoaded())
    {
        Unload();
    }
}

void Texture::Load()
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(_path.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    const vk::DeviceSize imageSize = static_cast<vk::DeviceSize>(texWidth) * texHeight * 4;

    Assert(pixels, "Could not load texture!");

    Graphics::_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    Graphics::CreateBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        Graphics::_stagingBuffer, Graphics::_stagingBufferMemory, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    void* data;
    VkResult result = vmaMapMemory(Graphics::_allocator, Graphics::_stagingBufferMemory, &data);
    Assert(result == VK_SUCCESS, "Failed to map texture memory!", { {"Error Code", static_cast<uint32_t>(result)} });
    memcpy(data, pixels, imageSize);
    vmaUnmapMemory(Graphics::_allocator, Graphics::_stagingBufferMemory);

    stbi_image_free(pixels);

    Graphics::CreateImage(texWidth, texHeight, Graphics::_mipLevels, vk::SampleCountFlagBits::e1, vk::Format::eR8G8B8A8Srgb,
        vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        _textureImage, _textureImageMemory);

    Graphics::TransitionImageLayout(_textureImage, vk::Format::eR8G8B8A8Srgb, Graphics::_mipLevels,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    Graphics::CopyBufferToImage(Graphics::_stagingBuffer, _textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    Graphics::GenerateMipmaps(_textureImage, vk::Format::eR8G8B8A8Srgb, texWidth, texHeight, Graphics::_mipLevels);

    vmaDestroyBuffer(Graphics::_allocator, Graphics::_stagingBuffer, Graphics::_stagingBufferMemory);
    CreateImageView();
}

void Texture::Unload()
{
	Graphics::_device.destroyImageView(_textureImageView, nullptr);
    vmaDestroyImage(Graphics::_allocator, _textureImage, _textureImageMemory);
}

bool Texture::IsLoaded() const
{
    return _textureImage;
}

void Texture::CreateImageView()
{
    _textureImageView = Graphics::CreateImageView(_textureImage, vk::Format::eR8G8B8A8Srgb,
        Graphics::_mipLevels, vk::ImageAspectFlagBits::eColor);
}

void Texture::CreateTextureSampler()
{
}
