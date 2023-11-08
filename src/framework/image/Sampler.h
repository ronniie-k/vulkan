#pragma once

#include "../rendering/Descriptor.h"

class Sampler
{
public:
	~Sampler();
	void create(vk::Filter filter, vk::SamplerAddressMode addressMode);
	void destroy();

	vk::Sampler handle;
private:
};