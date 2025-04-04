#pragma once

#include "ScriptFieldType.h"
#include <spdlog/spdlog.h>
#include <any>

namespace Mule
{
	class ScriptFieldInfo
	{
	public:
		ScriptFieldInfo()
			:
			mType(ScriptFieldType::None)
		{ }

		template<typename T>
		ScriptFieldInfo(ScriptFieldType type, const T value)
			:
			mType(type)
		{
			mValue = std::make_any<T>(value);
		}

		ScriptFieldInfo(ScriptFieldType type)
			:
			mType(type)
		{
			switch (type)
			{
			case ScriptFieldType::Int:
				mValue = std::make_any<int32_t>(0);
				break;
			case ScriptFieldType::Float:
				mValue = std::make_any<float>(0.f);
				break;
			case ScriptFieldType::Double:
				mValue = std::make_any<double>(0.0);
				break;
			case ScriptFieldType::String:
				mValue = std::make_any<std::string>("");
				break;
			case ScriptFieldType::Vector2:
				mValue = std::make_any<glm::vec2>(0.f, 0.f);
				break;
			case ScriptFieldType::Vector3:
				mValue = std::make_any<glm::vec3>(0.f, 0.f, 0.f);
				break;
			case ScriptFieldType::Vector4:
				mValue = std::make_any<glm::vec4>(0.f, 0.f, 0.f, 1.f);
				break;
			case ScriptFieldType::Entity:
				mValue = std::make_any<Entity>();
				break;
			default:
				break;
			}
		}

		template<typename T>
		T GetValue() const
		{
			if (mValue.type() != typeid(T))
			{
				SPDLOG_ERROR("Type mismatch: {} != {}", mValue.type().name(), typeid(T).name());
				return T();
			}
			return std::any_cast<T>(mValue);
		}

		template<typename T>
		void SetValue(const T& value)
		{
			if (mValue.type() != typeid(T))
			{
				SPDLOG_ERROR("Type mismatch: {} != {}", mValue.type().name(), typeid(T).name());
				return;
			}
			mValue = std::make_any<T>(value);
		}

		const ScriptFieldType& GetType() const
		{
			return mType;
		}

	private:
		std::any mValue;
		ScriptFieldType mType;
	};
}
