#pragma once
#include "../Asset.h"
#include "../../Graphics/Graphics.h"

class Texture : Asset
{
public:
	Texture(const std::filesystem::path& path);
	~Texture();
	void Load() override;
	void Unload() override;
	[[nodiscard]] bool IsLoaded() const override;
	friend class Graphics;
private:
	void CreateImageView();
	void CreateTextureSampler();
	vk::Image _textureImage;
	VmaAllocation _textureImageMemory{};
	vk::ImageView _textureImageView;
	const std::filesystem::path _path;
};