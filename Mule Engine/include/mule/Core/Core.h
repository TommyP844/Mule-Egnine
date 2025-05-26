#pragma once

#include <type_traits> // uint32_t
#include <string>

namespace Mule
{
#define MULE_ENUM_OPERATORS(type) \
	static type operator&(const type& lhs, const type& rhs) \
	{ \
		return static_cast<type>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)); \
	} \
	static type operator|(const type& lhs, const type& rhs) \
	{ \
		return static_cast<type>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs)); \
	} \

	template<typename T>
	static std::string ToString(T);

	template<typename T>
	static T FromString(const std::string&);

}