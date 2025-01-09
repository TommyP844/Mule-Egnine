#pragma once

#include "EditorState.h"
#include <string>

class IPanel
{
public:
	IPanel(const std::string& name) : mName(name), mIsOpen(true) {}
	virtual ~IPanel() {}

	virtual void OnUIRender() = 0;

	void SetContext(WeakRef<EditorState> editorState, WeakRef<Mule::ApplicationData> appData) { mEditorState = editorState; mApplicationData = appData; }
	void Open() { mIsOpen = true; }
	void Close() { mIsOpen = false; }
	void Toggle() { mIsOpen = !mIsOpen; }
	bool* OpenPtr() { return &mIsOpen; }
protected:
	bool mIsOpen;
	WeakRef<EditorState> mEditorState;
	WeakRef<Mule::ApplicationData> mApplicationData;
	const std::string mName;
};