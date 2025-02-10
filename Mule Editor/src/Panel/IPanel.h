#pragma once

#include "EditorContext.h"
#include <string>

class IPanel
{
public:
	IPanel(const std::string& name) : mName(name), mIsOpen(true) {}
	virtual ~IPanel() {}

	virtual void OnAttach() = 0;
	virtual void OnUIRender(float dt) = 0;
	virtual void OnEvent(Ref<IEditorEvent> event) = 0;

	void SetContext(WeakRef<EditorContext> editorContext, WeakRef<Mule::EngineContext> context) { mEditorContext = editorContext; mEngineContext = context; }
	void Open() { mIsOpen = true; }
	void Close() { mIsOpen = false; }
	void Toggle() { mIsOpen = !mIsOpen; }
	bool* OpenPtr() { return &mIsOpen; }
protected:
	bool mIsOpen;
	WeakRef<EditorContext> mEditorContext;
	WeakRef<Mule::EngineContext> mEngineContext;
	const std::string mName;
};