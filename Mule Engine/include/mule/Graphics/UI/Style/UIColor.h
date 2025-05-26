#pragma once

#include <glm/glm.hpp>
#include <type_traits>

namespace Mule
{

	struct UIColor 
	{
		glm::vec4 value;

		// Constructors
		UIColor() = default;
		UIColor(float r, float g, float b, float a) : value(r, g, b, a) {}
		UIColor(const glm::vec4& v) : value(v) {}
		operator glm::vec4& () { return value; }
		operator const glm::vec4& () const { return value; }

		// Accessors
		float& operator[](int i) { return value[i]; }
		const float& operator[](int i) const { return value[i]; }

		// Arithmetic operators
		UIColor operator+(const UIColor& other) const { return UIColor(value + other.value); }
		UIColor operator-(const UIColor& other) const { return UIColor(value - other.value); }
		UIColor operator*(const UIColor& other) const { return UIColor(value * other.value); }
		UIColor operator/(const UIColor& other) const { return UIColor(value / other.value); }

		UIColor operator*(float scalar) const { return UIColor(value * scalar); }
		UIColor operator/(float scalar) const { return UIColor(value / scalar); }

		// Compound assignment
		UIColor& operator+=(const UIColor& other) { value += other.value; return *this; }
		UIColor& operator-=(const UIColor& other) { value -= other.value; return *this; }
		UIColor& operator*=(const UIColor& other) { value *= other.value; return *this; }
		UIColor& operator/=(const UIColor& other) { value /= other.value; return *this; }

		UIColor& operator+=(const glm::vec4& other) { value += other; return *this; }
		UIColor& operator-=(const glm::vec4& other) { value -= other; return *this; }
		UIColor& operator*=(const glm::vec4& other) { value *= other; return *this; }
		UIColor& operator/=(const glm::vec4& other) { value /= other; return *this; }

		// Scalar operations

		UIColor& operator*=(float scalar) { value *= scalar; return *this; }
		UIColor& operator/=(float scalar) { value /= scalar; return *this; }

		// Comparison (optional)
		bool operator==(const UIColor& other) const { return value == other.value; }
		bool operator!=(const UIColor& other) const { return value != other.value; }
	};

	inline UIColor operator+(const glm::vec4& lhs, const UIColor& rhs) { return UIColor(lhs + rhs.value); }
	inline UIColor operator-(const glm::vec4& lhs, const UIColor& rhs) { return UIColor(lhs - rhs.value); }
	inline UIColor operator*(const glm::vec4& lhs, const UIColor& rhs) { return UIColor(lhs * rhs.value); }
	inline UIColor operator/(const glm::vec4& lhs, const UIColor& rhs) { return UIColor(lhs / rhs.value); }
}