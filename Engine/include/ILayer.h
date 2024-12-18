#pragma once

#include <string>

class Layer
{
public:
	Layer(std::string_view name) : mName(name) {}
	~Layer() {}

	virtual void OnUpdate(float dt) = 0;

	const std::string_view GetName() const { return mName; }
private:
	std::string mName;
};