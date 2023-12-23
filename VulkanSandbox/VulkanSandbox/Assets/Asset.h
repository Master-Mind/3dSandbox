#pragma once
#include <filesystem>

class Asset
{
	public:
	virtual void Load() = 0;
	virtual void Unload() = 0;
	 
	virtual bool IsLoaded() const = 0;
};
