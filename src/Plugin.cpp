#include "PCH.h"
#include "Dreams.h"
#include <atomic>
#include <mutex>
#include <random>

namespace {
constexpr std::uint32_t recordType = 0x4D525344; // DSRM; serialization ID and record type
// Random initial generation also invalidates suspended Papyrus stacks across process restarts.
std::atomic<int> session{static_cast<int>(std::random_device{}() & 0x3fffffff)};
std::atomic<bool> ready{false};
std::mutex stateMutex;
dreams::Active active;
std::uint32_t activeAV = 0;
float appliedDelta = 0;
std::mt19937 randomEngine{std::random_device{}()};

// Only game-thread tasks modify actor values. F4SE serialization callbacks also
// lock the state, so the co-save always describes exactly the modifier applied.
bool RemoveEffect() {
    if (!active.valid()) return true;
    auto* player = RE::PlayerCharacter::GetSingleton();
    auto* av = RE::TESForm::GetFormByID<RE::ActorValueInfo>(activeAV);
    if (!player || !av) {
        spdlog::error("Cannot remove dream: player or actor value {:08X} unavailable", activeAV);
        return false;
    }
    player->ModActorValue(RE::ACTOR_VALUE_MODIFIER::Perm, *av, -appliedDelta);
    active = {};
    activeAV = 0;
    appliedDelta = 0;
    return true;
}

void Tick(std::monostate) {
    const auto token = session.load();
    F4SE::GetTaskInterface()->AddTask([token] {
        if (!ready || token != session) return;
        std::scoped_lock lock(stateMutex);
        auto* calendar = RE::Calendar::GetSingleton();
        if (calendar && active.expired(calendar->GetHoursPassed())) RemoveEffect();
    });
}

int Session(std::monostate) { return session.load(); }

void Wake(std::monostate, int token) {
    F4SE::GetTaskInterface()->AddTask([token] {
        if (!ready || token != session) return;
        auto* player = RE::PlayerCharacter::GetSingleton();
        auto* calendar = RE::Calendar::GetSingleton();
        auto* messages = RE::MessageMenuManager::GetSingleton();
        if (!player || !calendar || !messages) return;
        std::scoped_lock lock(stateMutex);
        const auto now = static_cast<double>(calendar->GetHoursPassed());
        if (active.expired(now) && !RemoveEffect()) return;
        const auto index = dreams::select(
            std::uniform_int_distribution<unsigned>(0, 99)(randomEngine),
            std::uniform_int_distribution<std::size_t>(0, dreams::catalog.size()-1)(randomEngine));
        if (!index) return; // No dream: an unexpired previous effect remains.
        const auto& dream = dreams::catalog[*index];
        auto* av = RE::TESForm::GetFormByEditorID<RE::ActorValueInfo>(RE::BSFixedString(dream.actorValue));
        if (!av) {
            spdlog::error("Missing actor value {}; dream skipped", dream.actorValue);
            return;
        }
        if (!RemoveEffect()) return;
        player->ModActorValue(RE::ACTOR_VALUE_MODIFIER::Perm, *av, dream.amount);
        active = {static_cast<int>(*index), now + dreams::durationHours};
        activeAV = av->GetFormID();
        appliedDelta = dream.amount;
        spdlog::info("Dream {}: {}, expires at game hour {}", *index, dream.label, active.expires);
        messages->Create("The Sole Survivor Dreams", dream.text, nullptr,
            RE::WARNING_TYPES::kInGameMessage, "Wake up");
    });
}

void ClearDream(std::monostate) {
    ++session; // invalidate sleeping/queued dream requests, including uninstall waits
    const auto token = session.load();
    F4SE::GetTaskInterface()->AddTask([token] {
        if (!ready || token != session) return;
        std::scoped_lock lock(stateMutex);
        RemoveEffect();
    });
}

void Revert(const F4SE::SerializationInterface*) {
    ready = false;
    ++session;
    std::scoped_lock lock(stateMutex);
    // The engine restores player AVs from the chosen save. Never subtract the
    // old session's delta from the freshly loaded player or a new character.
    active = {};
    activeAV = 0;
    appliedDelta = 0;
}
void Save(const F4SE::SerializationInterface* io) {
    std::scoped_lock lock(stateMutex);
    if (!active.valid()) return;
    const auto index = static_cast<std::uint32_t>(active.index);
    if (!io->OpenRecord(recordType, 1) ||
        !io->WriteRecordData(&index, sizeof(index)) ||
        !io->WriteRecordData(&activeAV, sizeof(activeAV)) ||
        !io->WriteRecordData(&appliedDelta, sizeof(appliedDelta)) ||
        !io->WriteRecordData(&active.expires, sizeof(active.expires)))
        spdlog::error("Failed saving dream state; keep the previous good save");
}
void Load(const F4SE::SerializationInterface* io) {
    std::scoped_lock lock(stateMutex);
    std::uint32_t type, version, length;
    while (io->GetNextRecordInfo(type, version, length)) {
        if (type != recordType) continue;
        std::uint32_t index, avID;
        float delta;
        double expires;
        if (version != 1 || length != 20 ||
            io->ReadRecordData(&index, 4) != 4 || io->ReadRecordData(&avID, 4) != 4 ||
            io->ReadRecordData(&delta, 4) != 4 || io->ReadRecordData(&expires, 8) != 8 ||
            index >= dreams::catalog.size() || !std::isfinite(delta) || std::abs(delta) > 10 ||
            !std::isfinite(expires)) {
            spdlog::error("Invalid/unsupported dream co-save record; restore a known good save");
            continue;
        }
        const auto resolved = io->ResolveFormID(avID);
        if (!resolved) {
            spdlog::error("Unresolvable dream actor value; restore a known good save");
            continue;
        }
        active = {static_cast<int>(index), expires};
        activeAV = *resolved;
        appliedDelta = delta;
        // Modifier is already in the game's save. Do NOT reapply it here.
    }
}
bool Register(RE::BSScript::IVirtualMachine* vm) {
    vm->BindNativeMethod("TSSD_Native", "Session", Session);
    vm->BindNativeMethod("TSSD_Native", "Wake", Wake);
    vm->BindNativeMethod("TSSD_Native", "Tick", Tick);
    vm->BindNativeMethod("TSSD_Native", "ClearDream", ClearDream);
    return true;
}
constexpr F4SE::PluginVersionData Metadata() {
    F4SE::PluginVersionData data{};
    data.pluginVersion = 0x00010000;
    constexpr char name[] = "TheSoleSurvivorDreams";
    for (std::size_t i = 0; i < sizeof(name); ++i) data.name[i] = name[i];
    data.addressIndependence = F4SE::PluginVersionData::kAddressIndependence_Signatures;
    data.structureIndependence = F4SE::PluginVersionData::kStructureIndependence_1_10_980Layout |
        F4SE::PluginVersionData::kStructureIndependence_1_11_137Layout;
    return data;
}
}

extern "C" __declspec(dllexport) constinit F4SE::PluginVersionData F4SEPlugin_Version = Metadata();
extern "C" __declspec(dllexport) bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface* f4se) {
    if (f4se->IsEditor()) return false;
    F4SE::Init(f4se);
    auto logPath = F4SE::log::log_directory();
    if (!logPath) return false;
    *logPath /= "TheSoleSurvivorDreams.log";
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath->string(), true);
    spdlog::set_default_logger(std::make_shared<spdlog::logger>("TSSD", sink));
    spdlog::flush_on(spdlog::level::info);
    // The pinned library exposes SetUniqueID as non-const. Query the mutable
    // F4SE interface directly rather than casting away const on its accessor.
    auto* serialization = static_cast<F4SE::SerializationInterface*>(
        f4se->QueryInterface(F4SE::LoadInterface::kSerialization));
    if (!serialization || !F4SE::GetTaskInterface() || !F4SE::GetPapyrusInterface() || !F4SE::GetMessagingInterface()) return false;
    serialization->SetUniqueID(recordType);
    serialization->SetRevertCallback(Revert);
    serialization->SetSaveCallback(Save);
    serialization->SetLoadCallback(Load);
    if (!F4SE::GetPapyrusInterface()->Register(Register)) return false;
    return F4SE::GetMessagingInterface()->RegisterListener([](F4SE::MessagingInterface::Message* message) {
        if (message->type == F4SE::MessagingInterface::kPreLoadGame) {
            ready = false;
            ++session;
        } else if (message->type == F4SE::MessagingInterface::kPostLoadGame ||
                   message->type == F4SE::MessagingInterface::kNewGame) {
            ready = message->type == F4SE::MessagingInterface::kNewGame || message->data != nullptr;
            if (ready) Tick({});
        }
    });
}
