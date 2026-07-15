#define SDL_MAIN_HANDLED

#include "engine/bootstrap/bootstrapper.h"
#include "engine/config/config_service.h"
#include "engine/config/user_config_service.h"
#include "tests/support/test_assertions.h"

int main()
{
    const auto result = elysia::bootstrap::Bootstrapper::instance()->parse_runtime_settings();
    moonline::tests::require(result.success,"Bootstrapper must load AppConfig and UserConfig");
    auto* configs = elysia::config::ConfigService::instance();
    moonline::tests::require(!configs->is_initialized(),
        "Bootstrapper must not publish gameplay configuration before content loading");
    moonline::tests::require(elysia::config::UserConfigService::instance()->is_initialized(),
        "UserConfig startup behavior must remain integrated");
    configs->shutdown();
    elysia::config::UserConfigService::instance()->shutdown();
    return 0;
}
