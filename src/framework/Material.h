#pragma once

#include "framework/image/Texture.h"

struct Material
{
	Texture* albedo = nullptr;
	Texture* normal = nullptr;
	Texture* metallicRoughness = nullptr;
	Texture* occlusion = nullptr;
	Texture* emissive = nullptr;
};