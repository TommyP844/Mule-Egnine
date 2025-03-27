#pragma once

#include <string>

namespace Mule
{
	enum class ScriptFieldType
	{
		Unknown,
		Object,

		Bool,
		Byte,
		Int16,
		UInt16,
		Int32,
		UInt32,
		Int64,
		UInt64,
		Float,
		Double,
		Decimal,
		String,

		IntPtr,
		UIntPtr

	};

	static std::string GetFieldTypeName(ScriptFieldType type)
	{
		switch (type)
		{
		case Mule::ScriptFieldType::Bool: return "Bool";
		case Mule::ScriptFieldType::Int16: return "Int16";
		case Mule::ScriptFieldType::UInt16: return "UInt16";
		case Mule::ScriptFieldType::UInt32: return "UInt32";
		case Mule::ScriptFieldType::Int32: return "Int32";
		case Mule::ScriptFieldType::UInt64: return "UInt64";
		case Mule::ScriptFieldType::Int64: return "Int64";
		case Mule::ScriptFieldType::Float: return "Float";
		case Mule::ScriptFieldType::Double: return "Double";
		case Mule::ScriptFieldType::Decimal: return "Decimal";
		case Mule::ScriptFieldType::String: return "String";
		case Mule::ScriptFieldType::IntPtr: return "IntPtr";
		case Mule::ScriptFieldType::UIntPtr: return "UintPtr";
		case Mule::ScriptFieldType::Object: return "Object";
		}

		return "Unknown";
	}

	static ScriptFieldType GetFieldTypeFromString(const std::string& typeName)
	{
		if (typeName == "System.Boolean") return ScriptFieldType::Bool;
		if (typeName == "System.Byte") return ScriptFieldType::Byte;
		if (typeName == "System.Int16") return ScriptFieldType::Int16;
		if (typeName == "System.UInt16") return ScriptFieldType::UInt16;
		if (typeName == "System.Int32") return ScriptFieldType::Int32;
		if (typeName == "System.UInt32") return ScriptFieldType::UInt32;
		if (typeName == "System.Int64") return ScriptFieldType::Int64;
		if (typeName == "System.UInt64") return ScriptFieldType::UInt64;
		if (typeName == "System.Single") return ScriptFieldType::Float;
		if (typeName == "System.Double") return ScriptFieldType::Double;
		if (typeName == "System.Decimal") return ScriptFieldType::Decimal;
		if (typeName == "System.String") return ScriptFieldType::String;
		if (typeName == "System.IntPtr") return ScriptFieldType::IntPtr;
		if (typeName == "System.UIntPtr") return ScriptFieldType::UIntPtr;
		if (typeName.find("Mule.") != std::string::npos) return ScriptFieldType::Object;

		return ScriptFieldType::Unknown;
	}

}
