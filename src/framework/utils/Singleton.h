#pragma once

#include "NonCopyable.h"
#include "NonMoveable.h"

template<typename T>
class Singleton : public NonCopyable, public NonMoveable
{
public:
	static T& get()
	{
		static T instance;
		return instance;
	}

	virtual ~Singleton() = default;
protected:
	Singleton() = default;
};