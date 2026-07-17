#include "engine/config/user/user_config_store.h"
#include "engine/config/user_config_service.h"
#include "tests/support/test_assertions.h"

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <utility>

namespace
{
using moonline::tests::require;
using Data = elysia::bootstrap::UserConfigData;
void write(const std::filesystem::path& path,std::string_view text) { std::ofstream(path,std::ios::trunc) << text; }

class Handler final : public elysia::config::IUserConfigChangeHandler
{
public:
    std::expected<void,elysia::config::UserConfigFailure> apply_master_volume(int value) override
    {
        if (fail_master_value && value == *fail_master_value)
            return failure("master_volume");
        return {};
    }
    std::expected<void,elysia::config::UserConfigFailure> apply_music_volume(int) override { return {}; }
    std::expected<void,elysia::config::UserConfigFailure> apply_sound_volume(int) override { return {}; }
    std::expected<void,elysia::config::UserConfigFailure> apply_language(std::string_view value) override
    {
        if (!fail_language.empty() && value == fail_language)
            return failure("language");
        return {};
    }
    std::expected<void,elysia::config::UserConfigFailure> apply_target_fps(double) override { return {}; }
    std::expected<void,elysia::config::UserConfigFailure> apply_window_size(int,int) override { return {}; }
    std::expected<void,elysia::config::UserConfigFailure> apply_fullscreen(bool) override { return {}; }

    std::optional<int> fail_master_value;
    std::string fail_language;

private:
    static std::unexpected<elysia::config::UserConfigFailure> failure(std::string setting)
    {
        return std::unexpected(elysia::config::UserConfigFailure{
            elysia::config::UserConfigError::RuntimeApplyFailed,
            std::move(setting),
            "Injected runtime apply failure."
        });
    }
};
}

int main()
{
    const auto dir = std::filesystem::temp_directory_path() / "moonline_user_config_tests";
    std::filesystem::remove_all(dir); std::filesystem::create_directories(dir);
    const auto path = dir / "user_config.json";
    Data defaults; defaults.language="en";
    elysia::config::UserConfigStore store;

    write(path,R"({"window":{"width":1600},"localization":{"language":""}})");
    auto migrated = store.load(path,defaults);
    require(migrated && migrated->migrated,"v0 UserConfig must migrate");
    require(migrated->settings.window_width == 1600 && migrated->settings.language == "en","v0 must overlay defaults and normalize empty language");
    auto roundtrip = store.load(path,defaults);
    require(roundtrip && !roundtrip->migrated && roundtrip->settings == migrated->settings,"saved v1 UserConfig must round-trip");

    write(path,"{broken");
    write(path.string()+".bak",R"({"schema_version":1,"window":{"width":1920,"height":1080,"fullscreen":false},"render":{"fps":60,"vsync":true},"audio":{"master_volume":100,"music_volume":100,"sound_volume":100},"localization":{"language":"en"}})");
    auto recovered = store.load(path,defaults);
    require(recovered && recovered->recovered && recovered->settings.window_width == 1920,"backup must recover corrupt primary");

    write(path,R"({"schema_version":99})");
    require(!store.load(path,defaults),"future UserConfig version must stop loading without downgrade recovery");
    std::ifstream preserved(path); std::string text((std::istreambuf_iterator<char>(preserved)),{});
    require(text.find("99") != std::string::npos,"future UserConfig must be preserved");
    preserved.close();

    std::filesystem::remove(path);
    auto* service = elysia::config::UserConfigService::instance();
    require(service->initialize(defaults,path).has_value(),"UserConfigService must initialize from defaults");
    Handler handler; service->register_user_config_change_handler(handler);
    require(service->user_config().set_master_volume(50).has_value() && service->user_config().is_dirty(),"applied setting must mark UserConfig dirty");
    const auto vsync = service->user_config().set_vsync(false);
    require(vsync && *vsync == elysia::config::UserConfigApplyStatus::PendingRestart && service->user_config().restart_required(),"VSync change must retain restart semantics");
    require(service->save_user_config().has_value() && !service->user_config().is_dirty(),"save must mark UserConfig persisted");

    Data committed = service->user_config().snapshot();
    committed.window_width = 1600;
    committed.window_height = 900;
    committed.fullscreen = true;
    committed.audio.master_volume = 45;
    committed.audio.music_volume = 35;
    committed.audio.sound_volume = 25;
    committed.language = "zh_cn";
    const auto commit_result = service->apply_and_save_user_config(committed);
    require(commit_result.has_value() && service->user_config().snapshot() == committed
        && !service->user_config().is_dirty(),
        "batch commit must apply every setting and mark the committed snapshot persisted");

    const Data baseline = service->user_config().snapshot();
    Data rejected = baseline;
    rejected.window_width = 1920;
    rejected.audio.master_volume = 20;
    rejected.language = "unsupported";
    handler.fail_language = "unsupported";
    const auto rejected_result = service->apply_and_save_user_config(rejected);
    require(!rejected_result && !rejected_result.error().rollback_failure
        && service->user_config().snapshot() == baseline,
        "runtime apply failure must roll earlier settings back to the pre-commit snapshot");

    Data rollback_failure = baseline;
    rollback_failure.audio.master_volume = 10;
    rollback_failure.language = "still_unsupported";
    handler.fail_language = "still_unsupported";
    handler.fail_master_value = baseline.audio.master_volume;
    const auto rollback_failure_result = service->apply_and_save_user_config(rollback_failure);
    require(!rollback_failure_result && rollback_failure_result.error().rollback_failure
        && service->user_config().master_volume() == 10,
        "rollback failure must be reported while UserConfig keeps the actual runtime state");

    handler.fail_master_value.reset();
    handler.fail_language.clear();

    require(service->apply_and_save_user_config(baseline).has_value(),
        "the test fixture must recover to the persisted baseline");

    const elysia::config::UserConfigRuntimeState page_entry_state =
        service->user_config().runtime_state();
    require(service->user_config().set_target_fps(120.0).has_value(),
        "an external runtime change must be applicable while the settings page is open");
    Data rejected_after_external_change = service->user_config().snapshot();
    rejected_after_external_change.audio.master_volume = 15;
    rejected_after_external_change.language = "page_apply_failure";
    handler.fail_language = "page_apply_failure";
    const auto explicit_rollback_result = service->apply_and_save_user_config(
        rejected_after_external_change,
        page_entry_state);
    require(!explicit_rollback_result
        && !explicit_rollback_result.error().rollback_failure
        && service->user_config().runtime_state().settings == page_entry_state.settings
        && service->user_config().restart_required() == page_entry_state.restart_required,
        "a settings transaction must roll back to the page-entry runtime state, not the state at Save time");
    handler.fail_language.clear();

    const Data persistence_baseline = service->user_config().snapshot();
    const std::filesystem::path blocked_backup = path.string() + ".bak";
    std::filesystem::remove_all(blocked_backup);
    std::filesystem::create_directories(blocked_backup);
    write(blocked_backup / "blocker","prevent atomic backup replacement");

    Data persistence_failure = persistence_baseline;
    persistence_failure.audio.music_volume = 12;
    const auto persistence_failure_result =
        service->apply_and_save_user_config(persistence_failure);
    require(!persistence_failure_result
        && persistence_failure_result.error().cause.error
            == elysia::config::UserConfigError::SaveFailed
        && !persistence_failure_result.error().rollback_failure
        && service->user_config().snapshot() == persistence_baseline,
        "persistence failure must be visible and roll runtime settings back");
    std::filesystem::remove_all(blocked_backup);

    service->unregister_user_config_change_handler(handler); service->shutdown();
    std::filesystem::remove_all(dir);
}
