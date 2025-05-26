#pragma once

#include "Graphics/UI/Element/UIElementState.h"
#include "Graphics/UI/Element/UIElementType.h"

#include <unordered_map>

#define GET_STYLE_VAR_WITH_THEME(varName, type, varType) \
	const type& Get##varName(UIElementState state, WeakRef<varType> fallbackTheme) \
	{ \
		const auto& vars = GetStateVars(state); \
		if (vars.##varName.HasValue()) \
			return vars.##varName.GetValue(); \
		return fallbackTheme->GetStateVars(state).##varName.GetValue(); \
	} \


namespace Mule
{

	class UIBaseStyle
	{
	public:
		UIBaseStyle(UIElementType type)
			:
			mType(type)
		{
			mType = type;
		}

		UIElementType GetElementType() const { return mType; }

	private:
		UIElementType mType;
	};

	template<typename T, UIElementType type>
	class UIStyle : public UIBaseStyle
	{
	public:
		UIStyle()
			:
			UIBaseStyle(type)
		{}

		virtual ~UIStyle(){}

		T& GetStateVars(UIElementState state)
		{
			return mStyleVars[state];
		}

		const T& GetStateVars(UIElementState state) const
		{
			return mStyleVars.at(state);
		}
	private:
		std::unordered_map<UIElementState, T> mStyleVars;
	};
}
