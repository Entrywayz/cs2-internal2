#pragma once
#include <cmath>
#include <numbers>
#define M_PI 3.14159265358979323846

struct Vec2
{
	float x, y;

	Vec2() : x(0.0f), y(0.0f) {}
	Vec2(float _x, float _y) : x(_x), y(_y) {}
};

// Объявление QAngle_t заранее, чтобы его можно было использовать в Vec3
struct QAngle_t;

struct Vec3
{
	float x, y, z;

	Vec3 operator+(const Vec3& other) const {
		return { x + other.x, y + other.y, z + other.z };
	}

	Vec3 operator-(const Vec3& other) const {
		return { this->x - other.x, this->y - other.y, this->z - other.z };
	}

	Vec3 operator*(float& a) {
		return { x * a, y * a, z * a };
	}

	Vec3 RelativeAngle() const
	{
		return {
			std::atan2(-z, std::hypot(x, y)) * (180.0f / std::numbers::pi_v<float>),
			std::atan2(y, x) * (180.0f / std::numbers::pi_v<float>),
			0.0f
		};
	}

	Vec3 Normalized() const {
		float len = std::sqrt(x * x + y * y + z * z);

		// Предотвращаем деление на ноль
		if (len > 0.0f) {
			float invLen = 1.0f / len;
			return { x * invLen, y * invLen, z * invLen };
		}

		// Возвращаем нулевой вектор, если длина равна нулю
		return { 0.0f, 0.0f, 0.0f };
	}

	float Length() {
		return { std::sqrt(x * x + y * y + z * z) };
	}

	QAngle_t ToQAngle_t() const;

	float distanceTo(const Vec3& other) const {
		return std::sqrt(
			(x - other.x) * (x - other.x) +
			(y - other.y) * (y - other.y) +
			(z - other.z) * (z - other.z)
		);
	}

	Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
	Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
};

struct QAngle_t
{
	float pitch, yaw, roll;

	Vec3 ToVec3() const
	{
		float radPitch = pitch * (M_PI / 180.0f);
		float radYaw = yaw * (M_PI / 180.0f);

		float cosPitch = cos(radPitch);
		float x = cos(radYaw) * cosPitch;
		float y = sin(radYaw) * cosPitch;
		float z = -sin(radPitch);

		return Vec3(x, y, z);
	}

	QAngle_t operator-(QAngle_t& other) {
		return { this->pitch - other.pitch, this->yaw - other.yaw, this->roll - other.roll };
	}

	QAngle_t() : pitch(0.0f), yaw(0.0f), roll(0.0f) {}
	QAngle_t(float _pitch, float _yaw, float _roll) : pitch(_pitch), yaw(_yaw), roll(_roll) {}
};

inline QAngle_t Vec3::ToQAngle_t() const
{
	Vec3 angles;

	if (x == 0.0f && y == 0.0f)
	{
		angles.x = (z > 0.0f) ? 270.0f : 90.0f; // Pitch
		angles.y = 0.0f; // Yaw
	}
	else
	{
		angles.y = std::atan2(y, x) * (180.0f / std::numbers::pi_v<float>);

		float hyp = std::sqrt(x * x + y * y);
		angles.x = std::atan2(-z, hyp) * (180.0f / std::numbers::pi_v<float>);

		if (angles.y > 180.0f) angles.y -= 360.0f;
		if (angles.y < -180.0f) angles.y += 360.0f;

		if (angles.x > 89.0f) angles.x = 89.0f;
		if (angles.x < -89.0f) angles.x = -89.0f;
	}

	angles.z = 0.0f;

	return QAngle_t(angles.x, angles.y, angles.z);
}

bool WorldToScreen(Vec3 pos, Vec2& screen, float matrix[4][4]);

struct ResourceBinding_t
{
	void* pData;
};
template <typename T>
class CStrongHandle
{
public:
	operator T* () const
	{
		if (pBinding == nullptr)
			return nullptr;
		return static_cast<T*>(pBinding->pData);
	}
	T* operator->() const
	{
		if (pBinding == nullptr)
			return nullptr;
		return static_cast<T*>(pBinding->pData);
	}
	const ResourceBinding_t* pBinding;
};
