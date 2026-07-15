#include "engine/config/user_config_store.h"
#include "engine/config/user_config_service.h"
#include "tests/support/test_assertions.h"

#include <filesystem>
#include <fstream>

namespace
{
using moonline::tests::require;
using Data = elysia::bootstrap::UserConfigData;
void write(const std::filesystem::path& path,std::string_view text) { std::ofstream(path,std::ios::trunc) << text; }

class Handler final : public elysia::config::IUserConfigChangeHandler
{
public:
    std::expected<void,elysia::config::UserConfigFailure> apply_master_volume(int) override { return {}; }
    std::expected<void,elysia::config::UserConfigFailure> apply_music_volume(int) override { return {}; }
    std::expected<void,elysia::config::UserConfigFailure> apply_sound_volume(int) override { return {}; }
    std::expected<void,elysia::config::UserConfigFailure> apply_language(std::string_view) override { return {}; }
    std::expected<void,elysia::config::UserConfigFailure> apply_target_fps(double) override { return {}; }
    std::expected<void,elysia::config::UserConfigFailure> apply_window_size(int,int) override { return {}; }
    std::expected<void,elysia::config::UserConfigFailure> apply_fullscreen(bool) override { return {}; }
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
    service->unregister_user_config_change_handler(handler); service->shutdown();
    std::filesystem::remove_all(dir);
}
