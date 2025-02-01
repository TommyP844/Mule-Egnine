#pragma once

#include "IPanel.h"

class ComponentPanel : public IPanel
{
public:
	ComponentPanel() : IPanel("Component Panel") {}
	~ComponentPanel() {}

	virtual void OnAttach() override;
	virtual void OnUIRender(float dt) override;
	virtual void OnEvent(Ref<IEditorEvent> event) override {}

private:
	template<typename T>
	void DisplayComponent(const char* name, Mule::Entity e, std::function<void(T&)> func);

	void DisplayRow(const char* name, float dataColumnWidth = 200.f);
};

#include "ComponentPanel.inl"