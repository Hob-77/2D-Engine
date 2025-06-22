#pragma once
#include "Vec2.h"

struct AABB
{
	Vec2 min;
	Vec2 max;

	Vec2 center() const
	{
		return (min + max) * 0.5f;
	}

	// Create from center and half-sxtents (half width/height)
	static AABB FromCenterHalf(const Vec2& center, const Vec2& halfSize)
	{
		return { center - halfSize, center + halfSize };
	}

	// Create from position and full size (what sprites use)
	static AABB FromPositionSize(const Vec2& position, const Vec2& size)
	{
		return { position, position + size };
	}

	static AABB FromBottomCenter(const Vec2& bottomCenter, const Vec2& size)
	{
		Vec2 halfWidth(size.x * 0.5f, 0);
		Vec2 min = bottomCenter - halfWidth - Vec2(0, size.y);
		Vec2 max = bottomCenter + halfWidth;
		return { min, max };
	}

	bool intersects(const AABB& other) const
	{
		return min.x < other.max.x && max.x > other.min.x && min.y < other.max.y && max.y > other.min.y;
	}

	struct CollisionInfo
	{
		bool hit;
		Vec2 normal;
		float depth;
	};

	CollisionInfo getCollisionInfo(const AABB& other) const
	{
		CollisionInfo info{ false, Vec2(0,0), 0 };

		if (!intersects(other))
		{
			return info;
		}


		// Calculate overlap on each axis
		float xOverlap = std::min(max.x - other.min.x, other.max.x - min.x);
		float yOverlap = std::min(max.y - other.min.y, other.max.y - min.y);

		// Push out on smallest overlap axis
		if (xOverlap < yOverlap)
		{
			info.normal = (center().x < other.center().x) ? Vec2(-1, 0) : Vec2(1, 0);
			info.depth = xOverlap;
		}
		else {
			info.normal = (center().y < other.center().y) ? Vec2(0, -1) : Vec2(0, 1);
			info.depth = yOverlap;
		}

		info.hit = true;
		return info;
	}

};