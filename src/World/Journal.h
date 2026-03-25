#pragma once
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Journal
// Collects notes the player has read and auto-observations about key items.
// Persists on the Player struct across zone transitions.
// Max 32 entries -- plenty for a horror game.
// ---------------------------------------------------------------------------
struct JournalEntry {
	std::string title;   // shown in the left-hand list
	std::string body;    // shown in the right panel when selected
};

struct Journal {
	static constexpr int MAX_ENTRIES = 32;
	std::vector<JournalEntry> entries;

	int count() const { return (int)entries.size(); }
	bool empty() const { return entries.empty(); }

	// Add an entry, silently ignoring duplicates by title.
	void add(const std::string& title, const std::string& body) {
		if ((int)entries.size() >= MAX_ENTRIES) return;
		for (auto& e : entries)
			if (e.title == title) return;  // already collected
		entries.push_back({ title, body });
	}

	const JournalEntry* get(int index) const {
		if (index < 0 || index >= count()) return nullptr;
		return &entries[index];
	}
};