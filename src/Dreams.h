#pragma once
#include <array>
#include <cstddef>
#include <optional>
#include <cmath>

namespace dreams {
struct Dream { const char* actorValue; float amount; const char* label; const char* text; };
// Keep existing entries in order: indices are stored in F4SE co-saves. Append new dreams.
inline constexpr auto catalog = std::to_array<Dream>({
    {"Strength", 1.0F, "+1 Strength", R"dream([TODO: Write your +1 Strength dream here.])dream"}, // +1 Strength for 12 game hours
    {"Strength", -1.0F, "-1 Strength", R"dream([TODO: Write your -1 Strength dream here.])dream"}, // -1 Strength for 12 game hours
    {"Perception", 1.0F, "+1 Perception", R"dream([TODO: Write your +1 Perception dream here.])dream"}, // +1 Perception for 12 game hours
    {"Perception", -1.0F, "-1 Perception", R"dream([TODO: Write your -1 Perception dream here.])dream"}, // -1 Perception for 12 game hours
    {"Endurance", 1.0F, "+1 Endurance", R"dream([TODO: Write your +1 Endurance dream here.])dream"}, // +1 Endurance for 12 game hours
    {"Endurance", -1.0F, "-1 Endurance", R"dream([TODO: Write your -1 Endurance dream here.])dream"}, // -1 Endurance for 12 game hours
    {"Charisma", 1.0F, "+1 Charisma", R"dream([TODO: Write your +1 Charisma dream here.])dream"}, // +1 Charisma for 12 game hours
    {"Charisma", -1.0F, "-1 Charisma", R"dream([TODO: Write your -1 Charisma dream here.])dream"}, // -1 Charisma for 12 game hours
    {"Intelligence", 1.0F, "+1 Intelligence", R"dream([TODO: Write your +1 Intelligence dream here.])dream"}, // +1 Intelligence for 12 game hours
    {"Intelligence", -1.0F, "-1 Intelligence", R"dream([TODO: Write your -1 Intelligence dream here.])dream"}, // -1 Intelligence for 12 game hours
    {"Agility", 1.0F, "+1 Agility", R"dream([TODO: Write your +1 Agility dream here.])dream"}, // +1 Agility for 12 game hours
    {"Agility", -1.0F, "-1 Agility", R"dream([TODO: Write your -1 Agility dream here.])dream"}, // -1 Agility for 12 game hours
    {"Luck", 1.0F, "+1 Luck", R"dream([TODO: Write your +1 Luck dream here.])dream"}, // +1 Luck for 12 game hours
    {"Luck", -1.0F, "-1 Luck", R"dream([TODO: Write your -1 Luck dream here.])dream"}, // -1 Luck for 12 game hours
    {"Health", 10.0F, "+10 Health", R"dream([TODO: Write your +10 Health dream here.])dream"}, // +10 Health for 12 game hours
    {"Health", -10.0F, "-10 Health", R"dream([TODO: Write your -10 Health dream here.])dream"}, // -10 Health for 12 game hours
    {"ActionPoints", 10.0F, "+10 ActionPoints", R"dream([TODO: Write your +10 ActionPoints dream here.])dream"}, // +10 ActionPoints for 12 game hours
    {"ActionPoints", -10.0F, "-10 ActionPoints", R"dream([TODO: Write your -10 ActionPoints dream here.])dream"}, // -10 ActionPoints for 12 game hours
    {"DamageResist", 10.0F, "+10 DamageResist", R"dream([TODO: Write your +10 DamageResist dream here.])dream"}, // +10 DamageResist for 12 game hours
    {"DamageResist", -10.0F, "-10 DamageResist", R"dream([TODO: Write your -10 DamageResist dream here.])dream"}, // -10 DamageResist for 12 game hours
    {"EnergyResist", 10.0F, "+10 EnergyResist", R"dream([TODO: Write your +10 EnergyResist dream here.])dream"}, // +10 EnergyResist for 12 game hours
    {"EnergyResist", -10.0F, "-10 EnergyResist", R"dream([TODO: Write your -10 EnergyResist dream here.])dream"}, // -10 EnergyResist for 12 game hours
    {"RadResistExposure", 10.0F, "+10 RadResistExposure", R"dream([TODO: Write your +10 RadResistExposure dream here.])dream"}, // +10 RadResistExposure for 12 game hours
    {"RadResistExposure", -10.0F, "-10 RadResistExposure", R"dream([TODO: Write your -10 RadResistExposure dream here.])dream"}, // -10 RadResistExposure for 12 game hours
    {"ExperienceMult", 10.0F, "+10 % XP", R"dream([TODO: Write your +10 % XP dream here.])dream"}, // +10 % XP for 12 game hours
    {"ExperienceMult", -10.0F, "-10 % XP", R"dream([TODO: Write your -10 % XP dream here.])dream"}, // -10 % XP for 12 game hours
    {"CarryWeight", 10.0F, "+10 CarryWeight", R"dream([TODO: Write your +10 CarryWeight dream here.])dream"}, // +10 CarryWeight for 12 game hours
    {"CarryWeight", -10.0F, "-10 CarryWeight", R"dream([TODO: Write your -10 CarryWeight dream here.])dream"}, // -10 CarryWeight for 12 game hours
});
inline constexpr double durationHours = 12.0;
inline constexpr unsigned chancePercent = 100;
// Use a uniform roll in [0,99]; selection is uniform over every catalog entry.
inline std::optional<std::size_t> select(unsigned roll, std::size_t choice) {
    if (roll >= chancePercent || choice >= catalog.size()) return std::nullopt;
    return choice;
}
struct Active {
    int index = -1;
    double expires = 0;
    bool valid() const { return index >= 0 && static_cast<std::size_t>(index) < catalog.size() && std::isfinite(expires); }
    bool expired(double now) const { return valid() && now >= expires; }
};
}
