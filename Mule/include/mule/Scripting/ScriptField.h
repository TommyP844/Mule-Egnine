#pragma once

#include "Scripting/ScriptFieldType.h"

namespace Mule
{	
	enum class FieldAccessibility : uint32_t
	{
		Public,
		Private,
		Protected,
		Internal,
		ProtectedPublic,
		PrivateProtected
	};

	class ScriptField
	{
	public:
		ScriptField() = default;
		ScriptField(const std::string& name,
			ScriptFieldType type,
			FieldAccessibility accessibility)
			:
			Name(name),
			Type(type),
			Accessibility(accessibility)
		{ }

		std::string Name;
		ScriptFieldType Type;
		FieldAccessibility Accessibility;
	};
}
