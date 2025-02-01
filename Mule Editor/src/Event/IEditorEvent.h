#pragma once

enum class EditorEventType
{
	EditMaterial
};

class IEditorEvent
{
public:
	IEditorEvent(EditorEventType type) : mType(type), mIsHandled(false) {}
	virtual ~IEditorEvent() {}

	bool IsHandled() const { return mIsHandled; }
	void SetHandled() { mIsHandled = true; }
	EditorEventType GetEventType() const { return mType; }

private:
	bool mIsHandled;
	EditorEventType mType;
};
