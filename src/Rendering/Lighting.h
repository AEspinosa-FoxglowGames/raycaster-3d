#pragma once
#include <cmath>
#include "Entity.h"

// Deterministic layered sine flicker — no random state needed.
// Returns a [0,1] brightness multiplier.
inline float computeFlicker(LightType type, float timer)
{
	switch (type)
	{
	case LightType::HangingBulb:
	{
		// Layered sines at different speeds — feels organic, not stroby
		float slow = 0.80f + 0.20f * sinf(timer * 1.1f);
		float mid = 0.90f + 0.10f * sinf(timer * 8.3f + 0.7f);
		float fast = 0.95f + 0.05f * sinf(timer * 23.0f + 1.4f);
		// Occasional soft dip — sinf product goes low but never to zero
		float dip = 0.85f + 0.15f * sinf(timer * 4.7f) * sinf(timer * 3.1f);
		return slow * mid * fast * dip;
	}
	case LightType::WallSconce:
	{
		// Gentle breathing pulse
		float breath = 0.75f + 0.25f * sinf(timer * 2.1f);
		float drift = 1.0f + 0.08f * sinf(timer * 5.3f);
		return breath * drift;
	}
	case LightType::Flashlight:
	{
		// Very subtle wobble — the random cut-out is handled separately in main.
		// Simulates slight hand movement and battery inconsistency.
		float wobble = 0.96f + 0.04f * sinf(timer * 6.7f + 0.3f);
		float hum = 1.0f + 0.02f * sinf(timer * 31.0f);
		return wobble * hum;
	}
	case LightType::Ambient:
		return 1.0f;
	default:
		return 1.0f;
	}
}

// Call this once per frame for every light entity.
inline void updateEntityFlicker(Entity& e, float dt)
{
	if (e.lightType == LightType::None) return;
	e.flickerTimer += dt;
	// Offset each entity's phase by its id/address so they're not in sync
	float phase = e.flickerTimer + (float)(uintptr_t(&e) & 0xFF) * 0.04f;
	e.flickerValue = computeFlicker(e.lightType, phase);
}