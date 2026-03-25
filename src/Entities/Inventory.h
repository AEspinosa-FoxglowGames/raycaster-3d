#pragma once
#include <cstdint>
#include <array>

// ---------------------------------------------------------------------------
// ItemType
// ---------------------------------------------------------------------------
enum class ItemType : uint8_t {
	None = 0,
	Key = 1,
	Flashlight = 2,
	Baton = 3,
	FirstAidKit = 4,
	Logbook = 5,
	Fuse = 6,
	COUNT = 7,
};

inline const char* itemName(ItemType t) {
	switch (t) {
	case ItemType::Key:         return "Key";
	case ItemType::Flashlight:  return "Flashlight";
	case ItemType::Baton:       return "Baton";
	case ItemType::FirstAidKit: return "First Aid";
	case ItemType::Fuse:        return "Fuse";
	default:                    return "???";
	}
}

// ---------------------------------------------------------------------------
// Inventory
// Simple fixed-size slot array. MAX_SLOTS = 8.
// Persists on the Player struct across zone transitions.
// ---------------------------------------------------------------------------
struct Inventory {
	static constexpr int MAX_SLOTS = 8;

	std::array<ItemType, MAX_SLOTS> slots{};
	int _count = 0;

	int  count() const { return _count; }
	bool empty() const { return _count == 0; }
	bool full()  const { return _count >= MAX_SLOTS; }

	ItemType get(int index) const {
		if (index < 0 || index >= _count) return ItemType::None;
		return slots[index];
	}

	bool add(ItemType item) {
		if (_count >= MAX_SLOTS) return false;
		slots[_count++] = item;
		return true;
	}

	bool has(ItemType item) const {
		for (int i = 0; i < _count; ++i)
			if (slots[i] == item) return true;
		return false;
	}

	bool remove(ItemType item) {
		for (int i = 0; i < _count; ++i) {
			if (slots[i] == item) {
				for (int j = i; j < _count - 1; ++j)
					slots[j] = slots[j + 1];
				slots[--_count] = ItemType::None;
				return true;
			}
		}
		return false;
	}
};

inline const char* itemDescription(ItemType t)
{
	switch (t) {
	case ItemType::Key:        return "Opens a locked container or door.";
	case ItemType::Flashlight: return "A handheld light. Press F to toggle.";
	case ItemType::Baton:      return "A heavy baton.";
	case ItemType::FirstAidKit:return "Restores 50 HP when used.";
	case ItemType::Fuse:       return "A spare fuse. Fits a\nstandard fuse box.";
	case ItemType::Logbook:    return "A record of past events.";
	default:                   return "";
	}
}
